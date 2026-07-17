#include "NetworkCommon.h"


SteamNetworkingMicroseconds g_logTimeZero;

void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg) {

		SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
		printf("%10.6f %s\n", time * 1e-6, pszMsg);
		fflush(stdout);
		if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug) {
			fflush(stdout);
			fflush(stderr);
			NukeProcess(1);
		}
	
}
