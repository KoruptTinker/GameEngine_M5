#include "Networking/GameServer.h"
#include "main.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <signal.h>
#include <thread>

// Global server pointer for signal handling
GameServer *g_server = nullptr;

// Signal handler function
void signalHandler(int signal) {
  if (signal == SIGINT) {
    std::cout
        << "\nReceived SIGINT (Ctrl+C). Shutting down server gracefully..."
        << std::endl;
    if (g_server) {
      g_server->RequestStop();
    }
  }
}

int main() {
  std::cout << "Starting Space Invaders GameServer..." << std::endl;

  // Register signal handler for Ctrl+C
  signal(SIGINT, signalHandler);

  GameServer server;
  g_server = &server; // Set global pointer for signal handler

  // Initialize the server with headless game engine
  if (!server.Initialize("SpaceInvaders Server", 1000, 1000)) {
    std::cerr << "Failed to initialize server" << std::endl;
    return 1;
  }

  // Set up the player entity factory to spawn a PlayerBumper for each player
  server.SetPlayerSpawnEvent([&server](SDL_Renderer *renderer) -> Entity * {
    // Spawn near the bottom center
    // Slightly smaller bumper to match client/local sizing
    float width = 100.0f;
    float height = 55.0f;
    float screenWidth = 1000.0f;
    float x = (screenWidth - width) * 0.5f;
    float y = 900.0f;
    auto *player =
        new Player(x, y, width, height, server.GetRootTimeline(), renderer);
    player->setComponent("screenWidth", screenWidth);
    return player;
  });
  // Create hearts (3 hearts in top left corner)
  {
    auto *em = server.GetEntityManager();
    SDL_Renderer *renderer = server.GetRenderer();
    Timeline *tl = server.GetRootTimeline();

    const float heartSize = 32.0f;
    const float heartGap = 10.0f;
    const float startX = 20.0f;
    const float startY = 20.0f;

    for (int i = 0; i < 3; ++i) {
      float x = startX + i * (heartSize + heartGap);
      Heart *heart = new Heart(x, startY, heartSize, heartSize, tl, renderer);
      em->AddEntity(heart);
    }
  }

  // Create a simple grid of invaders at the top
  {
    auto *em = server.GetEntityManager();
    SDL_Renderer *renderer = server.GetRenderer();
    Timeline *tl = server.GetRootTimeline();

    const int rows = 3;
    const int cols = 8;
    const float invaderWidth = 32.0f;
    const float invaderHeight = 32.0f;
    const float gapX = 30.0f; // Increased horizontal spacing
    const float gapY = 25.0f; // Increased vertical spacing
    const float screenWidth = 1000.0f;
    const float totalWidth = cols * invaderWidth + (cols - 1) * gapX;
    const float startX = (screenWidth - totalWidth) * 0.5f;
    const float startY = 100.0f;

    for (int r = 0; r < rows; ++r) {
      for (int c = 0; c < cols; ++c) {
        float x = startX + c * (invaderWidth + gapX);
        float y = startY + r * (invaderHeight + gapY);
        int invaderType = (r + c) % 3;
        Invader *invader = new Invader(x, y, invaderWidth, invaderHeight,
                                       invaderType, tl, renderer);
        em->AddEntity(invader);
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
