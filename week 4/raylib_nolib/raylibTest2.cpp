
#define NOGDI             // Resolves ShowCursor conflict
#define NOUSER            // Resolves CloseWindow conflict


#include "raylib.h"

#include "NetworkClient.h" 


//const uint16 DEFAULT_SERVER_PORT = 27020;
//
//void PrintUsageAndExit(int rc = 1) {
//    fflush(stderr);
//    printf(
//        R"usage(Usage:
//    example client SERVER_ADDR
//    example server [--port PORT]
//)usage"
//);
//    fflush(stdout);
//    exit(rc);
//}


NetworkClient client;


void runClient() {
		
		// Application Loop
         const uint16_t  DEFAULT_SERVER_PORT = 27020;


         SteamNetworkingIPAddr addrServer;
         addrServer.Clear();
         
         // https://hexcalculator.org/ip-to-hex/
         // 127.0.0.1 converted to hex = 0x7F000001
         addrServer.SetIPv4(0x7F000001, DEFAULT_SERVER_PORT);
         

		client.Run(addrServer);
		
}


int main(void) {
    
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - input keys");

    Vector2 ballPosition = { (float)screenWidth / 2, (float)screenHeight / 2 };

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
   

    runClient();


    // Main game loop
    while (!WindowShouldClose()) {

        // Input
       
        if (IsKeyDown(KEY_RIGHT)) ballPosition.x += 2.0f;
        if (IsKeyDown(KEY_LEFT)) ballPosition.x -= 2.0f;
        if (IsKeyDown(KEY_UP)) ballPosition.y -= 2.0f;
        if (IsKeyDown(KEY_DOWN)) ballPosition.y += 2.0f;
        

        // Draw
        
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("move the ball with arrow keys", 10, 10, 20, DARKGRAY);

        DrawCircleV(ballPosition, 50, MAROON);

        EndDrawing();



        //client.receieveMessages();
        // Echo incoming message
        client.PollIncomingMessages();
        
        client.m_pClientSocketInterface->RunCallbacks();


        //client.sendMessages();

        // Instead of sending Messages we 
        // Just poll user input now.
        client.PollLocalUserInput();

        
        
    }

    client.g_bQuit = true;
    client.closeConnection();

    //
    // Shutdown Network
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

    // De-Initialization
    
    CloseWindow();        // Close window and OpenGL context
    

    return 0;
}
