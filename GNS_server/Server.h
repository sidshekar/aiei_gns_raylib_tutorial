#pragma once

#include <cstdint> //
class Server{

	public:

		Server();
		void Run();
	
	private:

		const uint16_t  DEFAULT_SERVER_PORT;
		bool g_bQuit;

};

