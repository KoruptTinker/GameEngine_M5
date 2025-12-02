#include "Core/GameEngine.h"
// #include <memory>
#include "main.h"

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  GameEngine engine;
  if (!engine.Initialize("Breakout Local", 1000, 1000, 1.0f)) {
    return 1;
  }

  InputManager* input = engine.GetInput();

  engine.GetRenderSystem()->SetScalingMode(ScalingMode::PROPORTIONAL);

  Timeline *rootTl = engine.GetRootTimeline();

  // Create PlayerBumper near bottom center
  float screenWidth = 1000.0f;
  float screenHeight = 1000.0f;
  // Slightly smaller bumper for better feel
  float bumperWidth = 220.0f;
  float bumperHeight = 55.0f;
  float bumperX = (screenWidth - bumperWidth) * 0.5f;
  float bumperY = 900.0f;
  PlayerBumper *bumper =
      new PlayerBumper(bumperX, bumperY, bumperWidth, bumperHeight, rootTl,
                       engine.GetRenderer());
  bumper->setComponent("screenWidth", screenWidth);

  // Create ball above the bumper
  // Slightly smaller ball
  float ballSize = 48.0f;
  float ballX = (screenWidth - ballSize) * 0.5f;
  float ballY = 700.0f;
  Ball *ball = new Ball(ballX, ballY, ballSize, ballSize, rootTl,
                        engine.GetRenderer());
  ball->setComponent("screenWidth", screenWidth);
  ball->setComponent("screenHeight", screenHeight);
  // Increase initial ball velocity by 2x in both directions
  ball->SetVelocity(300.0f, -500.0f);

  // Create bricks at the top in a centered, striped pattern
  const int rows = 6;
  const int cols = 8;
  // Slightly smaller bricks
  const float brickWidth = 96.0f;
  const float brickHeight = 32.0f;
  const float gapX = 10.0f;
  const float gapY = 10.0f;
  const float totalWidth = cols * brickWidth + (cols - 1) * gapX;
  const float startX = (screenWidth - totalWidth) * 0.5f;
  const float startY = 100.0f;

  auto *em = engine.GetEntityManager();

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      float x = startX + c * (brickWidth + gapX);
      float y = startY + r * (brickHeight + gapY);
      // Alternate brick type by row for a clear stripe pattern
      int brickType = r % 2;
      Brick *brick =
          new Brick(x, y, brickWidth, brickHeight, brickType, rootTl,
                    engine.GetRenderer());
      em->AddEntity(brick);
    }
  }

  // Add entities to the engine
  em->AddEntity(bumper);
  em->AddEntity(ball);

  engine.Run();

  SDL_Log("Cleaning up resources...");
  engine.Shutdown();
  SDL_Log("Shutdown complete. Exiting.");

  return 0;
}