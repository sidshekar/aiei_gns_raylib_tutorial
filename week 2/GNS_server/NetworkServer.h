#include "NetworkCommon.h"


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

	 std::vector< HSteamNetConnection> m_Clients;

	 static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo) {
		 s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
	 }

	 void SendStringToClient(HSteamNetConnection conn, const char* str) {
		m_pServerSocketInterface->SendMessageToConnection(conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr);
	 }
};