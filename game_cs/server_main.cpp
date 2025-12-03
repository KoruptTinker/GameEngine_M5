// server_main.cpp - Example GameServer usage
#include "Networking/GameServer.h"
#include <iostream>
#include <signal.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <SDL3/SDL.h>
#include "main.h"

// Global server pointer for signal handling
GameServer* g_server = nullptr;

// Signal handler function
void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nReceived SIGINT (Ctrl+C). Shutting down server gracefully..." << std::endl;
        if (g_server) {
            g_server->RequestStop();
        }
    }
}

int main() {
    std::cout << "Starting GameServer..." << std::endl;
    
    // Register signal handler for Ctrl+C
    signal(SIGINT, signalHandler);
    
    GameServer server;
    g_server = &server;  // Set global pointer for signal handler
    
    // Initialize the server with headless game engine
    if (!server.Initialize("Game Server", 320, 240)) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }

    Timeline *rootTimeline = server.GetRootTimeline();
    
    // Set up the player entity factory with proper Player class
    server.SetPlayerSpawnEvent([&server](SDL_Renderer* renderer) -> Entity* {
        // Load textures for the player
        SDL_Texture* idle = LoadTexture(renderer, "media/Idle_KG_1.bmp");
        SDL_Texture* runLeft = LoadTexture(renderer, "media/Walking_Left.bmp");
        SDL_Texture* runRight = LoadTexture(renderer, "media/Walking_Right.bmp");
        SDL_Texture* jumpLeft = LoadTexture(renderer, "media/Jump_Left.bmp");
        SDL_Texture* jumpRight = LoadTexture(renderer, "media/Jump_Right.bmp");
        return new Player(100, 100, idle, runLeft, runRight, jumpLeft, jumpRight, server.GetRootTimeline());
    });

    // Create the game world with platforms
    // Screen size: 1000x1000
    auto entityManager = server.GetEntityManager();
    
    // Ground platform - centered, smaller width (only 400 pixels wide)
    Platform *groundPlatform = new Platform(300, 900, 400, 50, Platform::PlatformType::STATIC, 0, 1000, rootTimeline);
    entityManager->AddEntity(groundPlatform);
    
    // Moving platforms that go from right to left at different heights and speeds
    Platform *movingPlatform1 = new Platform(800, 700, 200, 30, Platform::PlatformType::MOVING_HORIZONTAL, 150.0f, 1000, rootTimeline);
    Platform *movingPlatform2 = new Platform(500, 550, 180, 30, Platform::PlatformType::MOVING_HORIZONTAL, 120.0f, 1000, rootTimeline);
    Platform *movingPlatform3 = new Platform(200, 400, 220, 30, Platform::PlatformType::MOVING_HORIZONTAL, 180.0f, 1000, rootTimeline);
    Platform *movingPlatform4 = new Platform(900, 250, 190, 30, Platform::PlatformType::MOVING_HORIZONTAL, 140.0f, 1000, rootTimeline);
    
    entityManager->AddEntity(movingPlatform1);
    entityManager->AddEntity(movingPlatform2);
    entityManager->AddEntity(movingPlatform3);
    entityManager->AddEntity(movingPlatform4);
    
    // Start the server with publisher on port 5555 and pull socket on port 5556
    if (!server.StartServer(5555, 5556)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    std::cout << "Server started successfully!" << std::endl;
    std::cout << "Publisher port: 5555" << std::endl;
    std::cout << "Pull socket port: 5556" << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    // Run the server (this will run the game loop with networking)
    // The server's Run() method will check shouldStop internally
    server.Run();
    
    // Shutdown the server
    server.Shutdown();
    std::cout << "Server shutdown complete" << std::endl;
    
    return 0;
}