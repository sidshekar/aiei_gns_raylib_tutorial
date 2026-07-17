#include "NetworkServer.h"


NetworkServer* NetworkServer::s_pCallbackInstance = nullptr;

NetworkServer::NetworkServer() {
	
	s_pCallbackInstance = this;
}



void NetworkServer::RunServer(uint16 nPort, bool g_bQuit) {

	
	
// #ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE <-- check not required as we are using open source version.

	// Initialize socket
	SteamDatagramErrMsg errMsg;
	if (!GameNetworkingSockets_Init(nullptr, errMsg))
		FatalError("GameNetworkingSockets_Init failed.  %s", errMsg);


	// Output logtime
	SteamNetworkingMicroseconds g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();
	SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
	


	// Select instance to use.  For now we'll always use the default.
	// But we could use SteamGameServerNetworkingSockets() on Steam.
	m_pServerSocketInterface = SteamNetworkingSockets();

	// Start listening
	SteamNetworkingIPAddr serverLocalAddr;
	serverLocalAddr.Clear();
	serverLocalAddr.m_port = nPort;

	SteamNetworkingConfigValue_t opt;
	opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);

	m_hListenSock = m_pServerSocketInterface->CreateListenSocketIP(serverLocalAddr, 1, &opt);


	// Error checks
	if (m_hListenSock == k_HSteamListenSocket_Invalid)
		FatalError("Failed to listen on port %d", nPort);

	m_hPollGroup = m_pServerSocketInterface->CreatePollGroup();
	if (m_hPollGroup == k_HSteamNetPollGroup_Invalid)
		FatalError("Failed to listen on port %d", nPort);

	Printf("Server listening on port %d\n", nPort);

	// Poll for and handle messages
	while (!g_bQuit) {
				
		PollIncomingMessages(g_bQuit);

		// Poll Callbacks
		PollConnectionStateChanges();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// Close all the connections
	Printf("Closing connections...\n");
	
	//for (auto it : m_Clients) {
	for (auto it : m_mapClients){
		// Send them one more goodbye message.  Note that we also have the
		// connection close reason as a place to send final data.  However,
		// that's usually best left for more diagnostic/debug text not actual
		// protocol strings.
		
		//SendStringToClient(it, "Server is shutting down.  Goodbye.");
		SendStringToClient(it.first, "Server is shutting down.  Goodbye.");


		// Close the connection.  We use "linger mode" to ask SteamNetworkingSockets
		// to flush this out and close gracefully.
		
		//m_pServerSocketInterface->CloseConnection(it, 0, "Server Shutdown", true);

		m_pServerSocketInterface->CloseConnection(it.first, 0, "Server Shutdown", true);
	}
	
	//m_Clients.clear();
	m_mapClients.clear();

	m_pServerSocketInterface->CloseListenSocket(m_hListenSock);
	m_hListenSock = k_HSteamListenSocket_Invalid;

	m_pServerSocketInterface->DestroyPollGroup(m_hPollGroup);
	m_hPollGroup = k_HSteamNetPollGroup_Invalid;

}

/*
void NetworkServer::PollIncomingMessages(bool g_bQuit) {

	char temp[1024];

	while (!g_bQuit) {


		// RECEIEVE MESSAGES

		// Generally not even required since the server doesnt take 
		// Take any messages from the user!

		ISteamNetworkingMessage* pIncomingMsg = nullptr;
		int numMsgs = m_pServerSocketInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, &pIncomingMsg, 1);
		if (numMsgs == 0)
			break;
		if (numMsgs < 0)
			FatalError("Error checking for messages");


		// ONLY RECEIVE MESSAGES FROM OTHER CLIENTS

		assert(numMsgs == 1 && pIncomingMsg);
		auto itClient = std::find(m_Clients.begin(), m_Clients.end(), pIncomingMsg->m_conn);
		assert(itClient != m_Clients.end());

		// '\0'-terminate it to make it easier to parse
		// Assume it's a c-string and print it as-is
		std::string sCmd;

		// Populate a std::string with the data we received, assuming it's a character array
		sCmd.assign((const char*)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);

		// We don't need this anymore.
		pIncomingMsg->Release();

		const char* cmd = sCmd.c_str();
		Printf(cmd);


		// SEND MESSAGES

		// NEW
		// This is required since message received needs to be
		// relayed to all other clients.

		// Resend message to all other connected clients
		for (auto it : m_Clients) {
			// Don't send to client (make sure to test against the client from our collection)
			if (it == *itClient) { continue; }

			// Send to client
			SendStringToClient(it, cmd);
		}

	}

}

*/

// PollIncomingMessage based on Capnproto 

void NetworkServer::PollIncomingMessages(bool g_bQuit) {

	char temp[1024];

	while (!g_bQuit) {
		ISteamNetworkingMessage* pIncomingMsg = nullptr;
		int numMsgs = m_pServerSocketInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, &pIncomingMsg, 1);
		if (numMsgs == 0)
			break;
		if (numMsgs < 0)
			FatalError("Error checking for messages");
		assert(numMsgs == 1 && pIncomingMsg);
		auto itClient = m_mapClients.find(pIncomingMsg->m_conn);
		assert(itClient != m_mapClients.end());

		// CAPNPROTO - STEP 1 - Wrap the incoming message byte array in a kj:ArrayPtr object
		auto IncomingArr = kj::ArrayPtr<capnp::word>(reinterpret_cast<capnp::word*>(pIncomingMsg->m_pData), pIncomingMsg->m_cbSize / sizeof(capnp::word));
		
		// CAPNPROTO - STEP 2 - Initialize a flat array message reader with the above ArrayPtr as its constructor argument
		::capnp::FlatArrayMessageReader Message(IncomingArr);

		// CAPNPROTO - STEP 3 - Retrieve the message
		UserMessageType::Reader UserMessageRead = Message.getRoot<UserMessageType>();
		
		// CAPNPROTO - STEP 4 - Switch on the type of command that was set before transmission

		// SWITCH BASED ON MESSAGE TYPE RECEIVED. 

		switch (UserMessageRead.getCommand().which()) {
			
			// Not implemented.

			case UserMessageType::Command::LOGON:
				break;
		
			case UserMessageType::Command::SET_NICK: {

				auto MessageNewName = UserMessageRead.getCommand().getSetNick().getNewName();

				const char* nick = MessageNewName.cStr();
				while (isspace(*nick))
					++nick;

				// Let everybody else know they changed their name
				//sprintf(temp, "%s shall henceforth be known as %s", itClient->second.m_sNick.c_str(), nick);
				//SendStringToAllClients(temp, itClient->first);

				// Respond to client
				//sprintf(temp, "Ye shall henceforth be known as %s", nick);
				//SendStringToClient(itClient->first, temp);

				// Actually change their name
				SetClientNick(itClient->first, nick);
				break;
			}

			case UserMessageType::Command::SEND_CHAT:{

				// CAPNPROTO - STEP 5 - Retrieve chat contents
				auto MessageChatContents = UserMessageRead.getCommand().getSendChat().getContents();
				// Ordinary chat message, dispatch to everybody else
				sprintf(temp, "%s: %s", itClient->second.m_sNick.c_str(), MessageChatContents.cStr());
				SendStringToAllClients(temp, itClient->first);
				break;
			}

		}

		// We don't need this anymore.
		pIncomingMsg->Release();

	} // switch

}

void NetworkServer::SetClientNick(HSteamNetConnection hConn, const char* nick) {
	// Remember their nick
	m_mapClients[hConn].m_sNick = nick;

	// Set the connection name, too, which is useful for debugging
	m_pServerSocketInterface->SetConnectionName(hConn, nick);

}

//void NetworkServer::SendStringToClient(HSteamNetConnection conn, const char* str) {
//
//	m_pServerSocketInterface->SendMessageToConnection(conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr);
//}


void NetworkServer::SendStringToClient(HSteamNetConnection conn, const char* str) {

	// 1 - Create a message builder
	::capnp::MallocMessageBuilder messageBuilder;

	// 2 - Initialize the builder with our application message type
	UserMessageType::Builder userMessageTypeBuilder = messageBuilder.initRoot<UserMessageType>();

	// 3, 4 - Initialize our top-level object(command), union field (sendChat) and pupulate content of the message 
	userMessageTypeBuilder.initCommand().initSendChat().setContents(str);

	// 5 - Prepare message as a flat array of bytes
	const auto packedMessage = capnp::messageToFlatArray(messageBuilder);
	const auto packedBytes = packedMessage.asBytes();

	// 6 - Pass bytes to GameNetworkingSockets
	m_pServerSocketInterface->SendMessageToConnection(conn, packedBytes.begin(), (uint32)packedBytes.size(), k_nSteamNetworkingSend_Reliable, nullptr);
}

void NetworkServer::SendStringToAllClients(const char* str, HSteamNetConnection except) {

	for (auto& c : m_mapClients) {
		if (c.first != except)
			SendStringToClient(c.first, str);
	}

}

void NetworkServer::PollConnectionStateChanges() {


	//s_pCallbackInstance = this;
	m_pServerSocketInterface->RunCallbacks();
}


// CHANGES - In connecting, set random nick name, and mapClients used instead 

void NetworkServer::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {

		char temp[1024];

		// What's the state of the connection?
		switch (pInfo->m_info.m_eState) {

		case k_ESteamNetworkingConnectionState_None:
			// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
			break;

		case k_ESteamNetworkingConnectionState_ClosedByPeer:

		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			// Ignore if they were not previously connected.  (If they disconnected
			// before we accepted the connection.)
			if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected) {

				// Locate the client.  Note that it should have been found, because this
				// is the only codepath where we remove clients (except on shutdown),
				// and connection change callbacks are dispatched in queue order.
				
				//auto itClient = std::find(m_Clients.begin(), m_Clients.end(), pInfo->m_hConn);
				//assert(itClient != m_Clients.end());

				auto itClient = m_mapClients.find(pInfo->m_hConn);
				assert(itClient != m_mapClients.end());


				// Select appropriate log messages
				const char* pszDebugLogAction;
				if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
					pszDebugLogAction = "problem detected locally";
					sprintf(temp, "Alas, a client hath fallen into shadow.  (%s)", pInfo->m_info.m_szEndDebug);
				}
				else {
					// Note that here we could check the reason code to see if
					// it was a "usual" connection or an "unusual" one.
					pszDebugLogAction = "closed by peer";
					sprintf(temp, "A client hath departed");
				}

				// Spew something to our own log.  Note that because we put their nick
				// as the connection description, it will show up, along with their
				// transport-specific data (e.g. their IP address)
				Printf("Connection %s %s, reason %d: %s\n",
					pInfo->m_info.m_szConnectionDescription,
					pszDebugLogAction,
					pInfo->m_info.m_eEndReason,
					pInfo->m_info.m_szEndDebug
				);

				//m_Clients.erase(itClient);
				m_mapClients.erase(itClient);
			}
			else {
				assert(pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
			}

			// Clean up the connection.  This is important!
			// The connection is "closed" in the network sense, but
			// it has not been destroyed.  We must close it on our end, too
			// to finish up.  The reason information do not matter in this case,
			// and we cannot linger because it's already closed on the other end,
			// so we just pass 0's.
			m_pServerSocketInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			break;
		}

		case k_ESteamNetworkingConnectionState_Connecting:
		{
			// This must be a new connection
			//assert(std::find(m_Clients.begin(), m_Clients.end(), pInfo->m_hConn) == m_Clients.end());

			assert(m_mapClients.find(pInfo->m_hConn) == m_mapClients.end());

			Printf("Connection request from %s", pInfo->m_info.m_szConnectionDescription);

			// A client is attempting to connect
			
			// Try to accept the connection.
			if (m_pServerSocketInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK) {
				// This could fail.  If the remote host tried to connect, but then
				// disconnected, the connection may already be half closed.  Just
				// destroy whatever we have on our side.
				m_pServerSocketInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
				Printf("Can't accept connection.  (It was already closed?)");
				break;
			}

			// Assign the poll group
			if (!m_pServerSocketInterface->SetConnectionPollGroup(pInfo->m_hConn, m_hPollGroup)) {
				m_pServerSocketInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
				Printf("Failed to set poll group?");
				break;
			}

			/* 
				//OLD - before capnproto

				// Send them a welcome message
				sprintf(temp, "Welcome to the server");
				SendStringToClient(pInfo->m_hConn, temp);

				// Add them to the client list, using std::map wacky syntax
				m_Clients.push_back(pInfo->m_hConn);
				break;
			*/


			// Generate a random nick.  A random temporary nick
			// is really dumb and not how you would write a real chat server.
			// You would want them to have some sort of signon message,
			// and you would keep their client in a state of limbo (connected,
			// but not logged on) until them.  I'm trying to keep this example
			// code really simple.
			
			
				//char nick[64];
				//sprintf(nick, "BraveWarrior%d", 10000 + (rand() % 100000));

				// Send them a welcome message
				//sprintf(temp, "Welcome, stranger.  Thou art known to us for now as '%s'; upon thine command '/nick' we shall know thee otherwise.", nick);
				//SendStringToClient(pInfo->m_hConn, temp);

				// Also send them a list of everybody who is already connected
				if (m_mapClients.empty()) {
					SendStringToClient(pInfo->m_hConn, "Thou art utterly alone.");
				} else {
					sprintf(temp, "%d companions greet you:", (int)m_mapClients.size());
					SendStringToClient(pInfo->m_hConn, temp);
					for (auto& c : m_mapClients)
						SendStringToClient(pInfo->m_hConn, c.second.m_sNick.c_str());
				}

				// Add them to the client list, using std::map wacky syntax
				m_mapClients[pInfo->m_hConn];
				//SetClientNick(pInfo->m_hConn, nick);

				// Let everybody else know who they are for now
				//sprintf(temp, "Hark!  A stranger hath joined this merry host.  For now we shall call them '%s'", nick);
				//SendStringToAllClients(temp, pInfo->m_hConn);

			
			
			break;
		}

		case k_ESteamNetworkingConnectionState_Connected:
			// We will get a callback immediately after accepting the connection.
			// Since we are the server, we can ignore this, it's not news to us.
			break;

		default:
			// Silences -Wswitch
			break;
		}
}


