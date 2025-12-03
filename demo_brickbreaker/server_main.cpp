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

  signal(SIGINT, signalHandler);

  GameServer server;
  g_server = &server; 

  if (!server.Initialize("Breakout Server", 1000, 1000)) {
    std::cerr << "Failed to initialize server" << std::endl;
    return 1;
  }

  server.SetPlayerSpawnEvent([&server](SDL_Renderer *renderer) -> Entity * {
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

  // Create ball
  {
    float ballSize = 16.0f;
    float screenWidth = 1000.0f;
    float screenHeight = 1000.0f;
    float x = (screenWidth - ballSize) * 0.5f;
    float y = 900.0f - ballSize;
    Ball *ball = new Ball(x, y, ballSize, ballSize, server.GetRootTimeline(),
                          server.GetRenderer());
    ball->setComponent("screenWidth", screenWidth);
    ball->setComponent("screenHeight", screenHeight);

    ball->setComponent("attachedToBumper", true);
    ball->SetVelocity(0.0f, 0.0f);
    server.GetEntityManager()->AddEntity(ball);
  }

  // Create simple grid of bricks
  {
    auto *em = server.GetEntityManager();
    SDL_Renderer *renderer = server.GetRenderer();
    Timeline *tl = server.GetRootTimeline();

    const int rows = 6;
    const int cols = 8;
    const float brickWidth = 96.0f;
    const float brickHeight = 32.0f;
    const float gapX = 15.0f;
    const float gapY = 15.0f;
    const float screenWidth = 1000.0f;
    const float totalWidth = cols * brickWidth + (cols - 1) * gapX;
    const float startX = (screenWidth - totalWidth) * 0.5f;
    const float startY = 100.0f;

    for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
        float x = startX + c * (brickWidth + gapX);
        float y = startY + r * (brickHeight + gapY);
        int brickType = (r + c) % 2;
        Brick *brick =
            new Brick(x, y, brickWidth, brickHeight, brickType, tl, renderer);
        em->AddEntity(brick);
      }
    }
  }

  if (!server.StartServer(5555, 5556)) {
    std::cerr << "Failed to start server" << std::endl;
    return 1;
  }

  std::cout << "Server started successfully!" << std::endl;
  std::cout << "Publisher port: 5555" << std::endl;
  std::cout << "Pull socket port: 5556" << std::endl;
  std::cout << "Press Ctrl+C to stop the server" << std::endl;

  server.Run();

  server.Shutdown();
  std::cout << "Server shutdown complete" << std::endl;

  return 0;
}


