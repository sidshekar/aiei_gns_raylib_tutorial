#include "NetworkClient.h"

#include <iostream>

NetworkClient* NetworkClient::s_pCallbackInstance = nullptr;



NetworkClient::NetworkClient() {

	s_pCallbackInstance = this;
}

void NetworkClient::Run(const SteamNetworkingIPAddr& serverAddr) {

// New: Prepare for async user input
// 


	std::cout << "Enter a nick name" << std::endl;
	std::cin >> nickName;


	LocalUserInput_Init();


// Initialization


// Create client sockets
	SteamDatagramErrMsg errMsg;
	if (!GameNetworkingSockets_Init(nullptr, errMsg))
		FatalError("GameNetworkingSockets_Init failed.  %s", errMsg);


	SteamNetworkingMicroseconds g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();
	SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);

	
	// Select instance to use.  For now we'll always use the default.
	m_pClientSocketInterface = SteamNetworkingSockets();

	// Start connecting
	char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
	serverAddr.ToString(szAddr, sizeof(szAddr), true);
	Printf("Connecting to server at %s", szAddr);
	SteamNetworkingConfigValue_t opt;
	opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
	m_hConnection = m_pClientSocketInterface->ConnectByIPAddress(serverAddr, 1, &opt);
	if (m_hConnection == k_HSteamNetConnection_Invalid)
		FatalError("Failed to create connection");

	//while (!g_bQuit) {
		//
		// RECEIVE
		//
		
		//while (!g_bQuit) {
		//	ISteamNetworkingMessage* pIncomingMsg = nullptr;
		//	int numMsgs = m_pClientSocketInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
		//	// Nothing? Do nothing.
		//	if (numMsgs == 0)
		//		break;
		//	else if (numMsgs < 0)
		//		FatalError("Error checking for messages");
		//	else {
		//		// Just echo anything we get from the server
		//		fwrite(pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout);
		//		fputc('\n', stdout);

		//		// We don't need this anymore.
		//		pIncomingMsg->Release();
		//	}
		//}

		//
		// Poll Callbacks
		//

		
		//m_pClientSocketInterface->RunCallbacks();

		//
		// SEND
		//

		//std::string DebugMessage = "Client message";

		//// Send it to the server and let them parse it
		//m_pClientSocketInterface->SendMessageToConnection(m_hConnection, DebugMessage.c_str(), (uint32)DebugMessage.length(), k_nSteamNetworkingSend_Reliable, nullptr);
		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	//}

	//g_bQuit = true;

	// Close the connection gracefully.
	// We use linger mode to ask for any remaining reliable data
	// to be flushed out.  But remember this is an application
	// protocol on UDP.  See ShutdownSteamDatagramConnectionSockets
	
	//m_pClientSocketInterface->CloseConnection(m_hConnection, 0, "Goodbye", true);

}


// Gets replaced with PollIncomingMessages()

/*
	void NetworkClient::receieveMessages() {

		while (!g_bQuit) {
			ISteamNetworkingMessage* pIncomingMsg = nullptr;
			int numMsgs = m_pClientSocketInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
			// Nothing? Do nothing.
			if (numMsgs == 0)
				break;
			else if (numMsgs < 0)
				FatalError("Error checking for messages");
			else {
				// Just echo anything we get from the server
				fwrite(pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout);
				fputc('\n', stdout);

				// We don't need this anymore.
				pIncomingMsg->Release();
			}
		}

	}
*/

/*
	void NetworkClient::PollIncomingMessages() {

		while (!g_bQuit) {
			ISteamNetworkingMessage* pIncomingMsg = nullptr;
			int numMsgs = m_pClientSocketInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
			// Nothing? Do nothing.
			if (numMsgs == 0)
				break;
			else if (numMsgs < 0)
				FatalError("Error checking for messages");

			// Just echo anything we get from the server
			fwrite(pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout);
			fputc('\n', stdout);

			// We don't need this anymore.
			pIncomingMsg->Release();
		}
	}
*/

void NetworkClient::PollIncomingMessages() {

	while (!g_bQuit) {

		ISteamNetworkingMessage* pIncomingMsg = nullptr;
		int numMsgs = m_pClientSocketInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
		if (numMsgs == 0)
			break;
		if (numMsgs < 0)
			FatalError("Error checking for messages");

		// CAPNPROTO - Wrap the incoming message byte array in a kj:ArrayPtr object
		auto IncomingArr = kj::ArrayPtr<capnp::word>(reinterpret_cast<capnp::word*>(pIncomingMsg->m_pData), pIncomingMsg->m_cbSize / sizeof(capnp::word));
		// CAPNPROTO - Initialize a flat array message reader with the above ArrayPtr as its constructor argument
		::capnp::FlatArrayMessageReader Message(IncomingArr);

		// CAPNPROTO - Retrieve the message
		UserMessageType::Reader UserMessageRead = Message.getRoot<UserMessageType>();
		
		// CAPNPROTO - Switch on the type of command that was set before transmission
		switch (UserMessageRead.getCommand().which()) {

			case UserMessageType::Command::LOGON:
				break;
		
			case UserMessageType::Command::SET_NICK: 
				break;

			case UserMessageType::Command::SEND_CHAT:
			
				// CAPNPROTO - Retrieve chat contents
				// Just echo anything we get from the server
				fwrite(UserMessageRead.getCommand().getSendChat().getContents().cStr(), 1, UserMessageRead.getCommand().getSendChat().getContents().size(), stdout);
				fputc('\n', stdout);
				break;
		
		}

		// We don't need this anymore.
		pIncomingMsg->Release();
	}

}

// Gets reaplced with PollLocalUserInput

/* 

	void NetworkClient::sendMessages() {
		std::string DebugMessage = "Client message";

		// Send it to the server and let them parse it
		m_pClientSocketInterface->SendMessageToConnection(m_hConnection, DebugMessage.c_str(), (uint32)DebugMessage.length(), k_nSteamNetworkingSend_Reliable, nullptr);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	}
*/


void NetworkClient::PollLocalUserInput() {

	std::string cmd;

	while (!g_bQuit && LocalUserInput_GetNext(cmd)) {

		// Check for known commands

		if (strcmp(cmd.c_str(), "/quit") == 0) {
			g_bQuit = true;
			Printf("Disconnecting from chat server");

			// Close the connection gracefully.
			// We use linger mode to ask for any remaining reliable data
			// to be flushed out.  But remember this is an application
			// protocol on UDP.  See ShutdownSteamDatagramConnectionSockets
			m_pClientSocketInterface->CloseConnection(m_hConnection, 0, "Goodbye", true);
			break;
		} else if (strcmp(cmd.c_str(), "/nick") == 0){
		
			SendNickName(nickName.c_str());
		}
		else {
			
			//m_pClientSocketInterface->SendMessageToConnection(m_hConnection, cmd.c_str(), (uint32)cmd.length(), k_nSteamNetworkingSend_Reliable, nullptr);

			SendString(cmd.c_str());
		}
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}


void NetworkClient::SendNickName(const char* str) {

	::capnp::MallocMessageBuilder Message;
	UserMessageType::Builder UserMessagePack = Message.initRoot<UserMessageType>();
	
	// 3, 4
	UserMessagePack.initCommand().initSetNick().setNewName(nickName);

	// 5
	const auto PackedMessage = capnp::messageToFlatArray(Message);
	const auto PackedBytes = PackedMessage.asBytes();

	// 6
	m_pClientSocketInterface->SendMessageToConnection(m_hConnection, PackedBytes.begin(), (uint32)PackedBytes.size(), k_nSteamNetworkingSend_Reliable, nullptr);

}

void NetworkClient::SendString(const char* str) {
	// 1
	::capnp::MallocMessageBuilder Message;

	// 2
	UserMessageType::Builder UserMessagePack = Message.initRoot<UserMessageType>();
	// 3, 4
	UserMessagePack.initCommand().initSendChat().setContents(str);

	// 5
	const auto PackedMessage = capnp::messageToFlatArray(Message);
	const auto PackedBytes = PackedMessage.asBytes();

	// 6
	m_pClientSocketInterface->SendMessageToConnection(m_hConnection, PackedBytes.begin(), (uint32)PackedBytes.size(), k_nSteamNetworkingSend_Reliable, nullptr);

}



void NetworkClient::closeConnection() {

	m_pClientSocketInterface->CloseConnection(m_hConnection, 0, "Goodbye", true);

}


void NetworkClient::LocalUserInput_Init() {

	s_pThreadUserInput = new std::thread([&]() {

		while (!g_bQuit) {
			char szLine[4000];
			if (!fgets(szLine, sizeof(szLine), stdin)) {
				// Well, you would hope that you could close the handle
				// from the other thread to trigger this.  Nope.
				if (g_bQuit)
					return;
				g_bQuit = true;
				Printf("Failed to read on stdin, quitting\n");
				break;
			}

			mutexUserInputQueue.lock();
			queueUserInput.push(std::string(szLine));
			mutexUserInputQueue.unlock();
		}
	});
}

bool NetworkClient::LocalUserInput_GetNext(std::string& result) {

	bool got_input = false;
	mutexUserInputQueue.lock();
	while (!queueUserInput.empty() && !got_input) {
		result = queueUserInput.front();
		queueUserInput.pop();
		ltrim(result);
		rtrim(result);
		got_input = !result.empty(); // ignore blank lines
	}
	mutexUserInputQueue.unlock();
	return got_input;
}


void NetworkClient::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {

		assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

		// What's the state of the connection?
		switch (pInfo->m_info.m_eState) {
			
			case k_ESteamNetworkingConnectionState_None:
					// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
					break;

			case k_ESteamNetworkingConnectionState_ClosedByPeer:

			case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			{
				 // TODO:: FIX -> g_bQuit = true;

				// Print an appropriate message
				if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting) {
					// Note: we could distinguish between a timeout, a rejected connection,
					// or some other transport problem.
					Printf("We sought the remote host, yet our efforts were met with defeat.  (%s)", pInfo->m_info.m_szEndDebug);
				}
				else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
					Printf("Alas, troubles beset us; we have lost contact with the host.  (%s)", pInfo->m_info.m_szEndDebug);
				}
				else {
					// NOTE: We could check the reason code for a normal disconnection
					Printf("The host hath bidden us farewell.  (%s)", pInfo->m_info.m_szEndDebug);
				}

				// Clean up the connection.  This is important!
				// The connection is "closed" in the network sense, but
				// it has not been destroyed.  We must close it on our end, too
				// to finish up.  The reason information do not matter in this case,
				// and we cannot linger because it's already closed on the other end,
				// so we just pass 0's.
				m_pClientSocketInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
				m_hConnection = k_HSteamNetConnection_Invalid;
				break;
			}

			case k_ESteamNetworkingConnectionState_Connecting:
				// We will get this callback when we start connecting.
				// We can ignore this.
				break;

			case k_ESteamNetworkingConnectionState_Connected: {
				
				Printf("Connected to server OK");

				SendNickName(nickName.c_str());

				break;
			}
			default:
				// Silences -Wswitch
				break;
		
		}
}


