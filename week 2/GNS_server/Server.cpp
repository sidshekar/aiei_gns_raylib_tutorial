#include "Server.h"

#include "NetworkServer.h"

Server::Server():  DEFAULT_SERVER_PORT(27020), g_bQuit(false) {
	
	SteamNetworkingIPAddr addrServer;
	addrServer.Clear();
}

void Server::Run() {

	NetworkServer server;
	server.RunServer(static_cast<uint16>(DEFAULT_SERVER_PORT), g_bQuit);

	//
	// Shutdown
	//

	// Give connections time to finish up.  This is an application layer protocol
	// here, it's not TCP.  Note that if you have an application and you need to be
	// more sure about cleanup, you won't be able to do this.  You will need to send
	// a message and then either wait for the peer to close the connection, or
	// you can pool the connection to see if any reliable data is pending.
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	GameNetworkingSockets_Kill();

	// Ug, why is there no simple solution for portable, non-blocking console user input?
	// Just nuke the process
	//LocalUserInput_Kill();
	NukeProcess(0);

}
