#include "NetworkCommon.h"

#define KJ_NO_EXCEPTIONS 1
#include <capnp/message.h>
#include <capnp/serialize-packed.h>

#include "UserMessage.capnp.h"

class NetworkClient {


public:
	NetworkClient();
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
	void Run(const SteamNetworkingIPAddr& serverAddr);

	ISteamNetworkingSockets* m_pClientSocketInterface;
	bool g_bQuit = false;


	//void receieveMessages();
	void PollIncomingMessages(); //<-- gets redefined
	
	//void sendMessages();
	
	// Either send commands or send messages
	void PollLocalUserInput();

	// used to send messages through cpnproto
	void SendString(const char* str);

	void SendNickName(const char* str);
	


	void closeConnection();

private:
	
	std::string nickName;

	static  NetworkClient* s_pCallbackInstance;
	
	HSteamNetConnection m_hConnection;

	static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo) {

		s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
	}	


	std::mutex mutexUserInputQueue;
	std::queue< std::string > queueUserInput;

	std::thread* s_pThreadUserInput = nullptr;

	// Creates a new thread to get the input.
	// Creates a look and constantly pushes the inputs in the queue.
	// Queue is a shared resource. Used by this function and GetNext().
	// This function pushes into the queue and the other function pops.
	void LocalUserInput_Init();

	// Get the next item in the queue.
	bool LocalUserInput_GetNext(std::string& result);


};

