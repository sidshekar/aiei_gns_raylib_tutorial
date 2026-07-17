#include "NetworkCommon.h"

#define KJ_NO_EXCEPTIONS 1
#include <capnp/message.h>
#include <capnp/serialize-packed.h>

#include "UserMessage.capnp.h"

#include <vector>


class NetworkServer {

public:
	NetworkServer();
	
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
	void RunServer(uint16 nPort, bool g_bQuit);


private:

	 static  NetworkServer* s_pCallbackInstance;
	 ISteamNetworkingSockets* m_pServerSocketInterface;
	 HSteamListenSocket m_hListenSock;
	 HSteamNetPollGroup m_hPollGroup;

	 //std::vector< HSteamNetConnection> m_Clients;

	 // now: a client struct that stores just a nickname (for now)
	 struct Client_t {
		 std::string m_sNick;
	 };

	 std::map< HSteamNetConnection, Client_t > m_mapClients;

	 
	 void SetClientNick(HSteamNetConnection hConn, const char* nick);

	 static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo) {
		 s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
	 }

	 void SendStringToClient(HSteamNetConnection conn, const char* str); // <- redefined

	 void SendStringToAllClients(const char* str, HSteamNetConnection except = k_HSteamNetConnection_Invalid);

	 void PollIncomingMessages(bool g_bQuit); // <- Redefined.

	 void PollConnectionStateChanges();



};