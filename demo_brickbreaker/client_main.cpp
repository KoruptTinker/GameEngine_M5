#include "Networking/GameClient.h"
#include "main.h"
#include <iostream>
#include <string>

int main() {
  std::cout << "Starting Breakout GameClient..." << std::endl;

  GameClient client;

  if (!client.Initialize("Breakout Client", 1000, 1000, 1.0f)) {
    std::cerr << "Failed to initialize client" << std::endl;
    return 1;
  }

  client.RegisterEntity("PlayerBumper", [&client]() -> Entity * {
    return new PlayerBumper(0, 0, 300, 75, client.GetRootTimeline(),
                            client.GetRenderer());
  });
  client.RegisterEntity("Ball", [&client]() -> Entity * {
    return new Ball(0, 0, 64, 64, client.GetRootTimeline(),
                    client.GetRenderer());
  });
  client.RegisterEntity("Brick_1", [&client]() -> Entity * {
    return new Brick(0, 0, 384, 128, 0, client.GetRootTimeline(),
                     client.GetRenderer());
  });

  client.RegisterEntity("Brick_2", [&client]() -> Entity * {
    return new Brick(0, 0, 384, 128, 1, client.GetRootTimeline(),
                     client.GetRenderer());
  });

  client.RegisterEntity("Heart", [&client]() -> Entity * {
    return new Heart(0, 0, 32, 32, client.GetRootTimeline(),
                     client.GetRenderer());
  });

  std::string serverAddress = "localhost";
  int publisherPort = 5555;
  int pullPort = 5556;

  client.GetInput()->AddAction("MOVE_LEFT", SDL_SCANCODE_A);
  client.GetInput()->AddAction("MOVE_RIGHT", SDL_SCANCODE_D);
  client.GetInput()->AddAction("LAUNCH_BALL", SDL_SCANCODE_SPACE);
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
      << "Use A/D to move the bumper, SPACE to launch the ball, ESC to exit"
      << std::endl;

  // Run the client (this will handle input, networking, and rendering)
  client.Run();

  client.Shutdown();
  std::cout << "Client shutdown complete" << std::endl;

  return 0;
}
