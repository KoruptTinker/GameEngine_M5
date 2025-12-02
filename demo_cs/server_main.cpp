// server_main.cpp - Breakout-style GameServer
#include "Networking/GameServer.h"
#include <iostream>
#include <signal.h>
#include <atomic>
#include <thread>
#include <chrono>
#include "main.h"

// Global server pointer for signal handling
GameServer *g_server = nullptr;

// Signal handler function
void signalHandler(int signal) {
  if (signal == SIGINT) {
    std::cout << "\nReceived SIGINT (Ctrl+C). Shutting down server gracefully..."
              << std::endl;
    if (g_server) {
      g_server->RequestStop();
    }
  }
}

int main() {
  std::cout << "Starting Breakout GameServer..." << std::endl;

  // Register signal handler for Ctrl+C
  signal(SIGINT, signalHandler);

  GameServer server;
  g_server = &server; // Set global pointer for signal handler

  // Initialize the server with headless game engine
  if (!server.Initialize("Breakout Server", 1000, 1000)) {
    std::cerr << "Failed to initialize server" << std::endl;
    return 1;
  }

  // Set up the player entity factory to spawn a PlayerBumper for each player
  server.SetPlayerSpawnEvent([&server](SDL_Renderer *renderer) -> Entity * {
    // Spawn near the bottom center
    // Slightly smaller bumper to match client/local sizing
    float width = 220.0f;
    float height = 55.0f;
    float screenWidth = 1000.0f;
    float x = (screenWidth - width) * 0.5f;
    float y = 900.0f;
    auto *bumper =
        new PlayerBumper(x, y, width, height, server.GetRootTimeline(),
                         renderer);
    bumper->setComponent("screenWidth", screenWidth);
    return bumper;
  });

  // Create the ball (single shared ball) using the same boundaries as the client
  {
    // Slightly smaller ball
    float ballSize = 32.0f;
    float screenWidth = 1000.0f;
    float screenHeight = 1000.0f;
    float x = (screenWidth - ballSize) * 0.5f;
    // Start roughly where the bumper will be and let the ball "ride" on top
    float y = 900.0f - ballSize;
    Ball *ball = new Ball(x, y, ballSize, ballSize, server.GetRootTimeline(),
                          server.GetRenderer());
    // Ensure ball boundary logic uses the client's screen dimensions
    ball->setComponent("screenWidth", screenWidth);
    ball->setComponent("screenHeight", screenHeight);
    // Start attached to the bumper and stationary; launch will be triggered
    // by a client action (space bar) handled on the server.
    ball->setComponent("attachedToBumper", true);
    ball->SetVelocity(0.0f, 0.0f);
    server.GetEntityManager()->AddEntity(ball);
  }

  // Create a simple grid of bricks at the top
  {
    auto *em = server.GetEntityManager();
    SDL_Renderer *renderer = server.GetRenderer();
    Timeline *tl = server.GetRootTimeline();

    // More bricks in a centered, striped pattern
    const int rows = 6;
    const int cols = 8;
    // Slightly smaller bricks
    const float brickWidth = 96.0f;
    const float brickHeight = 32.0f;
    const float gapX = 10.0f;
    const float gapY = 10.0f;
    const float screenWidth = 1000.0f;
    const float totalWidth = cols * brickWidth + (cols - 1) * gapX;
    const float startX = (screenWidth - totalWidth) * 0.5f;
    const float startY = 100.0f;

    for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
        float x = startX + c * (brickWidth + gapX);
        float y = startY + r * (brickHeight + gapY);
        // Alternate brick type in a checkerboard pattern for a more
        // visually interesting layout.
        int brickType = (r + c) % 2;
        Brick *brick =
            new Brick(x, y, brickWidth, brickHeight, brickType, tl, renderer);
        em->AddEntity(brick);
      }
    }
  }

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