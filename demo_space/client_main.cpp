#include "Networking/GameClient.h"
#include "main.h"
#include <iostream>
#include <string>

int main() {
  std::cout << "Starting Space Invaders GameClient..." << std::endl;

  GameClient client;

  // Initialize the client window to match the game resolution
  if (!client.Initialize("Space Invader Client", 1000, 1000, 1.0f)) {
    std::cerr << "Failed to initialize client" << std::endl;
    return 1;
  }

  // Register entity factory functions for this game
  client.RegisterEntity("Player", [&client]() -> Entity * {
    return new Player(0, 0, 300, 75, client.GetRootTimeline(),
                      client.GetRenderer());
  });
  client.RegisterEntity("Invader_0", [&client]() -> Entity * {
    // brickType will be overwritten by network state; default to 0
    return new Invader(0, 0, 384, 128, 0, client.GetRootTimeline(),
                       client.GetRenderer());
  });
  client.RegisterEntity("Invader_1", [&client]() -> Entity * {
    // brickType will be overwritten by network state; default to 0
    return new Invader(0, 0, 384, 128, 1, client.GetRootTimeline(),
                       client.GetRenderer());
  });
  client.RegisterEntity("Invader_2", [&client]() -> Entity * {
    // brickType will be overwritten by network state; default to 0
    return new Invader(0, 0, 384, 128, 2, client.GetRootTimeline(),
                       client.GetRenderer());
  });
  client.RegisterEntity("Bullet", [&client]() -> Entity * {
    return new Bullet(0, 0, 10, 10, nullptr, client.GetRootTimeline(),
                      client.GetRenderer());
  });

  client.RegisterEntity("Heart", [&client]() -> Entity * {
    return new Heart(0, 0, 32, 32, client.GetRootTimeline(),
                     client.GetRenderer());
  });

  // Connect to the server (assuming server is running on localhost)
  std::string serverAddress = "localhost";
  int publisherPort = 5555;
  int pullPort = 5556;

  // Map input actions for controlling the player
  client.GetInput()->AddAction("MOVE_LEFT", SDL_SCANCODE_A);
  client.GetInput()->AddAction("MOVE_RIGHT", SDL_SCANCODE_D);
  client.GetInput()->AddAction("SHOOT", SDL_SCANCODE_SPACE);
  client.GetInput()->AddChordAction("DASH_LEFT",
                                    {SDL_SCANCODE_LSHIFT, SDL_SCANCODE_A});
  client.GetInput()->AddChordAction("DASH_RIGHT",
                                    {SDL_SCANCODE_LSHIFT, SDL_SCANCODE_D});
  std::cout << "Connecting to server at " << serverAddress << ":"
            << publisherPort << "/" << pullPort << std::endl;

  if (!client.ConnectToServer(serverAddress, publisherPort, pullPort)) {
    std::cerr << "Failed to connect to server" << std::endl;
    return 1;
  }

  std::cout << "Client connected successfully!" << std::endl;
  std::cout << "Client ID: " << client.GetClientId() << std::endl;
  std::cout
      << "Use A/D to move, SPACE to shoot, Shift+A/D to dash, ESC to exit"
      << std::endl;

  // Run the client (this will handle input, networking, and rendering)
  client.Run();

  client.Shutdown();
  std::cout << "Client shutdown complete" << std::endl;

  return 0;
}
