#include "NetworkCommon.h"



class NetworkClient {


public:
	NetworkClient();
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
	void Run(const SteamNetworkingIPAddr& serverAddr);

	ISteamNetworkingSockets* m_pClientSocketInterface;
	bool g_bQuit = false;


	void receieveMessages();
	void sendMessages();
	void closeConnection();

private:
	

	static  NetworkClient* s_pCallbackInstance;
	
	HSteamNetConnection m_hConnection;

	static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo) {

		s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
	}	
};

