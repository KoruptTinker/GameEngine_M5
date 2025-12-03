// client_main.cpp - Example GameClient usage
#include "Networking/GameClient.h"
#include <iostream>
#include <string>
#include <SDL3/SDL.h>
#include "main.h"

int main(int argc, char* argv[]) {
    // Parse command line arguments for time scale
    float timeScale = 1.0f; // Default value
    
    if (argc > 1) {
        try {
            timeScale = std::stof(argv[1]);
            // Validate time scale range (same as GameEngine validation)
            if (timeScale < 0.5f || timeScale > 2.0f) {
                std::cerr << "Time scale must be between 0.5 and 2.0. Using default 1.0f" << std::endl;
                timeScale = 1.0f;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid time scale argument. Using default 1.0f" << std::endl;
            timeScale = 1.0f;
        }
    }
    
    std::cout << "Starting GameClient with time scale: " << timeScale << std::endl;
    std::cout << "Usage: " << argv[0] << " [time_scale]" << std::endl;
    std::cout << "  time_scale: Float between 0.5 and 2.0 (default: 1.0)" << std::endl;
    
    GameClient client;
    
    // Register entity factory functions

    // Initialize the client with the parsed time scale
    if (!client.Initialize("Game Client", 1000, 1000, timeScale)) {
        std::cerr << "Failed to initialize client" << std::endl;
        return 1;
    }

    // Register entity factory functions for the game entities
    client.RegisterEntity("Player", [&client]() -> Entity* { 
        // Load textures for the player
        SDL_Texture* idle = LoadTexture(client.GetRenderer(), "media/Idle_KG_1.bmp");
        SDL_Texture* runLeft = LoadTexture(client.GetRenderer(), "media/Walking_Left.bmp");
        SDL_Texture* runRight = LoadTexture(client.GetRenderer(), "media/Walking_Right.bmp");
        SDL_Texture* jumpLeft = LoadTexture(client.GetRenderer(), "media/Jump_Left.bmp");
        SDL_Texture* jumpRight = LoadTexture(client.GetRenderer(), "media/Jump_Right.bmp");
        return new Player(100, 100, idle, runLeft, runRight, jumpLeft, jumpRight, client.GetRootTimeline()); 
    });
    
    client.RegisterEntity("Platform", [&client]() -> Entity* { 
        // Load textures for the platform
        SDL_Texture* platformTexture = LoadTexture(client.GetRenderer(), "media/cartooncrypteque_platform_basicground_idle.bmp");
        Entity* platform = new Platform(0, 0, 200, 20, Platform::PlatformType::STATIC, 0, 1000, client.GetRootTimeline()); 
        // Set up texture data for the platform
        Texture platformTextureData = {platformTexture, 1, 1, 200, 20, true};
        platform->SetTexture(0, &platformTextureData);
        return platform;
    });
    
    // Connect to the server (assuming server is running on localhost)
    std::string serverAddress = "localhost";
    int publisherPort = 5555;
    int pullPort = 5556;
    // Set up input actions for the Player entity
    client.GetInput()->AddAction("MOVE_LEFT", SDL_SCANCODE_A);
    client.GetInput()->AddAction("MOVE_LEFT", SDL_SCANCODE_LEFT);
    client.GetInput()->AddAction("MOVE_RIGHT", SDL_SCANCODE_D);
    client.GetInput()->AddAction("MOVE_RIGHT", SDL_SCANCODE_RIGHT);
    client.GetInput()->AddAction("JUMP", SDL_SCANCODE_SPACE);
    client.GetInput()->AddAction("PAUSE", SDL_SCANCODE_P);
    client.GetInput()->AddAction("SPEED_DOWN", SDL_SCANCODE_I);
    client.GetInput()->AddAction("SPEED_UP", SDL_SCANCODE_O);
    client.GetInput()->AddAction("RESET_SPEED", SDL_SCANCODE_U);
    std::cout << "Connecting to server at " << serverAddress << ":" << publisherPort << "/" << pullPort << std::endl;
    
    if (!client.ConnectToServer(serverAddress, publisherPort, pullPort)) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }
    
    std::cout << "Client connected successfully!" << std::endl;
    std::cout << "Client ID: " << client.GetClientId() << std::endl;
    std::cout << "Game Controls:" << std::endl;
    std::cout << "  WASD or Arrow Keys: Move left/right" << std::endl;
    std::cout << "  SPACE: Jump" << std::endl;
    std::cout << "  P: Pause/Unpause timeline" << std::endl;
    std::cout << "  I/O: Slow down/Speed up timeline" << std::endl;
    std::cout << "  U: Reset timeline speed" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
    std::cout << "The client will send input to server and receive game state updates" << std::endl;
    
    // Run the client (this will handle input, networking, and rendering)
    client.Run();
    
    client.Shutdown();
    std::cout << "Client shutdown complete" << std::endl;
    
    return 0;
}