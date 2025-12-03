#pragma once
#include <cmath>
#include <iostream>

#include "Core/GameEngine.h"
#include "Core/Render.h"
#include "Entities/Entity.h"
#include "Math/vec2.h"

// Forward declarations
class Heart;
class Brick;

class PlayerBumper : public Entity {
  inline static MemoryPool *MemPool = nullptr;

public:
  void *operator new(size_t size) {
    if (!PlayerBumper::MemPool)
      PlayerBumper::MemPool = new MemoryPool(sizeof(PlayerBumper), 16);
    int sl_id = PlayerBumper::MemPool->alloc();
    if (sl_id == -1)
      return nullptr;
    return PlayerBumper::MemPool->getPtr(sl_id);
  }

  void operator delete(void *ptr) {
    PlayerBumper::MemPool->freeSlot(PlayerBumper::MemPool->getSlot(ptr));
  }

  PlayerBumper(float x, float y, float w, float h, Timeline *tl,
               SDL_Renderer *renderer = nullptr)
      : Entity(x, y, w, h, tl) {

    entityType = "PlayerBumper";
    setComponent("screenWidth", 1800.0f);
    EnablePhysics(false);
    EnableCollision(false, false);
    setComponent("lastFrameTime", Uint32(0));
    setComponent("animationDelay", 100);
    setComponent("dashLeftActive", false);
    setComponent("dashRightActive", false);
    if (renderer) {
      SDL_Texture *tex1 = LoadTexture(renderer, "media/bumper_1.bmp");
      SDL_Texture *tex2 = LoadTexture(renderer, "media/bumper_2.bmp");
      rendering.textures[0] = Texture{
          .sheet = tex1,
          .num_frames_x = 1,
          .num_frames_y = 1,
          .frame_width = 485,
          .frame_height = 128,
          .loop = true,
      };
      rendering.textures[1] = Texture{
          .sheet = tex2,
          .num_frames_x = 1,
          .num_frames_y = 1,
          .frame_width = 485,
          .frame_height = 128,
          .loop = true,
      };
      SetTextureState(0);
      setComponent("textureState", 0);
    }
  }

  void Update(float deltaTime, InputManager *input,
              EntityManager *entitySpawner) override {
    setComponent("dashLeftActive", false);
    setComponent("dashRightActive", false);

    if (entitySpawner && !hasComponent("entityManagerPtr")) {
      setComponent("entityManagerPtr", (void *)entitySpawner);
    }

    Uint32 lastFrameTime = getComponent<Uint32>("lastFrameTime");
    int animationDelay = getComponent<int>("animationDelay");
    int textureState = getComponent<int>("textureState");
    lastFrameTime += (Uint32)(deltaTime * 1000); 
    if (lastFrameTime >= (Uint32)animationDelay) {
      textureState = !textureState;
      SetTextureState(textureState);
      setComponent("textureState", textureState);
      lastFrameTime = 0;
    }
    setComponent("lastFrameTime", lastFrameTime);

    static bool pKeyWasPressed = false;
    bool pKeyIsPressed = input->IsKeyPressed(SDL_SCANCODE_P);

    if (pKeyIsPressed && !pKeyWasPressed) {
      if (timeline->getState() == Timeline::State::PAUSE) {
        timeline->setState(Timeline::State::RUN);
      } else {
        timeline->setState(Timeline::State::PAUSE);
      }
    }
    pKeyWasPressed = pKeyIsPressed;

    float screenWidth = getComponent<float>("screenWidth");
    if (position.x < 0.0f) {
      position.x = 0.0f;
    }
    if (position.x + dimensions.x > screenWidth) {
      position.x = screenWidth - dimensions.x;
    }
  }

  void OnActivity(const std::string &actionName) override {
    constexpr float moveSpeed = 300.0f;
    constexpr float dashSpeed = 600.0f;

    if (actionName == "MOVE_LEFT") {
      bool dashLeftActive = getComponent<bool>("dashLeftActive");
      if (!dashLeftActive) {
        SetVelocityX(-moveSpeed);
      }
    } else if (actionName == "MOVE_RIGHT") {
      bool dashRightActive = getComponent<bool>("dashRightActive");
      if (!dashRightActive) {
        SetVelocityX(moveSpeed);
      }
    } else if (actionName == "DASH_LEFT") {
      SetVelocityX(-dashSpeed);
      setComponent("dashLeftActive", true);
    } else if (actionName == "DASH_RIGHT") {
      SetVelocityX(dashSpeed);
      setComponent("dashRightActive", true);
    } else if (actionName == "LAUNCH_BALL") {
      EntityManager *entityMgr = nullptr;
      if (hasComponent("entityManagerPtr")) {
        entityMgr = reinterpret_cast<EntityManager *>(
            getComponent<void *>("entityManagerPtr"));
      }
      if (entityMgr) {
        Entity *ball = nullptr;
        for (Entity *e : entityMgr->getEntityVectorRef()) {
          if (e && e->entityType == "Ball") {
            ball = e;
            break;
          }
        }
        if (ball) {
          bool attachedToBumper = false;
          if (ball->hasComponent("attachedToBumper")) {
            attachedToBumper = ball->getComponent<bool>("attachedToBumper");
          }
          if (attachedToBumper) {
            ball->setComponent("attachedToBumper", false);
            ball->SetVelocity(GetVelocityX(), -500.0f);
          }
        }
      }
    } else {
      SetVelocityX(0);
    }
  }
};

class Ball : public Entity {

  inline static MemoryPool *MemPool = nullptr;

public:
  void *operator new(size_t size) {
    if (!Ball::MemPool)
      Ball::MemPool = new MemoryPool(sizeof(Ball), 16);
    int sl_id = Ball::MemPool->alloc();
    if (sl_id == -1)
      return nullptr;
    return Ball::MemPool->getPtr(sl_id);
  }

  void operator delete(void *ptr) {
    Ball::MemPool->freeSlot(Ball::MemPool->getSlot(ptr));
  }

  Ball(float x, float y, float w, float h, Timeline *tl,
       SDL_Renderer *renderer = nullptr)
      : Entity(x, y, w, h, tl) {

    EnablePhysics(false);
    EnableCollision(false, false);

    entityType = "Ball";
    setComponent("screenWidth", 1800.0f);
    setComponent("screenHeight", 1000.0f);
    setComponent("justCollided", false);
    setComponent("rendererPtr", (void *)renderer);
    if (renderer) {
      SDL_Texture *tex = LoadTexture(renderer, "media/ball.bmp");
      rendering.textures[0] = Texture{
          .sheet = tex,
          .num_frames_x = 1,
          .num_frames_y = 1,
          .frame_width = 128,
          .frame_height = 128,
          .loop = true,
      };
    }
  }

  void ResetGame(EntityManager *entitySpawner);

  void Update(float deltaTime, InputManager *input,
              EntityManager *entitySpawner) override {
    (void)deltaTime;
    (void)input;
    setComponent("justCollided", false);
    bool attachedToBumper = false;
    if (hasComponent("attachedToBumper")) {
      attachedToBumper = getComponent<bool>("attachedToBumper");
    }

    Entity *bumper = nullptr;
    if (entitySpawner) {
      for (Entity *e : entitySpawner->getEntityVectorRef()) {
        if (e && e->entityType == "PlayerBumper") {
          bumper = e;
          break;
        }
      }
    }

    if (attachedToBumper && bumper) {
      position.x =
          bumper->position.x + (bumper->dimensions.x - dimensions.x) * 0.5f;
      position.y = bumper->position.y - dimensions.y;
      SetVelocity(0.0f, 0.0f);
      return;
    }

    float screenWidth = getComponent<float>("screenWidth");
    float screenHeight = getComponent<float>("screenHeight");

    bool hitLeftBoundary = position.x <= 0;
    bool hitRightBoundary = position.x + dimensions.x >= screenWidth;
    bool hitTopBoundary = position.y <= 0;
    bool hitBottomBoundary = position.y + dimensions.y >= screenHeight;

    if (hitLeftBoundary || hitRightBoundary) {
      float currentVelX = GetVelocityX();
      SetVelocityX(-currentVelX);
      if (hitLeftBoundary) {
        position.x = 0;
      } else {
        position.x = screenWidth - dimensions.x;
      }
    }

    if (hitTopBoundary) {
      float currentVelY = GetVelocityY();
      SetVelocityY(-currentVelY);
      position.y = 0;
    }

    if (hitBottomBoundary) {
      bool gameOver = false;
      if (entitySpawner) {
        Entity *heartToRemove = nullptr;
        int heartCount = 0;
        for (Entity *e : entitySpawner->getEntityVectorRef()) {
          if (e && e->entityType == "Heart" &&
              !e->getComponent<bool>("destroyed")) {
            heartToRemove = e;
            heartCount++;
          }
        }
        if (heartToRemove) {
          heartToRemove->setComponent("destroyed", true);
          heartCount--;
        }
        if (heartCount <= 0) {
          gameOver = true;
          ResetGame(entitySpawner);
        }
      }

      if (bumper) {
        position.x =
            bumper->position.x + (bumper->dimensions.x - dimensions.x) * 0.5f;
        position.y = bumper->position.y - dimensions.y;
      } else {
        position.x = (screenWidth - dimensions.x) * 0.5f;
        position.y = (screenHeight - dimensions.y) * 0.5f;
      }

      SetVelocityY(0.0f);
      SetVelocityX(0.0f);
      setComponent("attachedToBumper", true);
    }
  }

  void OnCollision(Entity *other, CollisionData *data) override {
    if (!other || !data)
      return;

    if (other->entityType == "PlayerBumper") {
      float ballCenterX = position.x + dimensions.x * 0.5f;
      float ballCenterY = position.y + dimensions.y * 0.5f;
      float bumperCenterX = other->position.x + other->dimensions.x * 0.5f;
      float bumperCenterY = other->position.y + other->dimensions.y * 0.5f;

      float offset = ballCenterX - bumperCenterX;

      float halfBumperWidth = other->dimensions.x * 0.5f;
      float normalizedOffset = offset / halfBumperWidth;

      if (normalizedOffset < -1.0f)
        normalizedOffset = -1.0f;
      if (normalizedOffset > 1.0f)
        normalizedOffset = 1.0f;

      constexpr float maxBounceAngle = 75.0f;
      float bounceAngle = normalizedOffset * maxBounceAngle;

      float angleRad = bounceAngle * M_PI / 180.0f;

      float currentVelX = GetVelocityX();
      float currentVelY = GetVelocityY();
      float currentSpeed =
          sqrtf(currentVelX * currentVelX + currentVelY * currentVelY);

      if (currentSpeed < 1.0f) {
        currentSpeed = 300.0f;
      }
      float newVelX = currentSpeed * sinf(angleRad);
      float newVelY = -currentSpeed * cosf(angleRad);

      SetVelocityX(newVelX);
      SetVelocityY(newVelY);
    } else if (other->entityType == "Brick_1" ||
               other->entityType == "Brick_2") {
      if (other->getComponent<bool>("collidedAlready")) {
        return;
      }
      other->setComponent("collidedAlready", true);
      if (getComponent<bool>("justCollided")) {
        return;
      }
      if (data->normal.y != 0) {
        SetVelocityY(-GetVelocityY());
      } else {
        SetVelocityX(-GetVelocityX());
      }
    }
  }
};

class Brick : public Entity {
  inline static MemoryPool *MemPool = nullptr;

public:
  void *operator new(size_t size) {
    if (!Brick::MemPool)
      Brick::MemPool = new MemoryPool(sizeof(Brick), 128);
    int sl_id = Brick::MemPool->alloc();
    if (sl_id == -1)
      return nullptr;
    return Brick::MemPool->getPtr(sl_id);
  }

  void operator delete(void *ptr) {
    Brick::MemPool->freeSlot(Brick::MemPool->getSlot(ptr));
  }

  Brick(float x, float y, float w, float h, int brickType, Timeline *tl,
        SDL_Renderer *renderer = nullptr)
      : Entity(x, y, w, h, tl) {
    EnableCollision(false, true);
    setComponent("destroyed", false);
    setComponent("state", 0);
    setComponent("brokenCooldown", 0.0f);
    setComponent("collidedAlready", false);
    entityType = "Brick";
    if (renderer) {
      SDL_Texture *tex;
      SDL_Texture *tex_broken;
      switch (brickType) {
      case 0:
        entityType = "Brick_1";
        tex = LoadTexture(renderer, "media/brick_1.bmp");
        tex_broken = LoadTexture(renderer, "media/brick_1_broken.bmp");
        break;
      case 1:
        entityType = "Brick_2";
        tex = LoadTexture(renderer, "media/brick_2.bmp");
        tex_broken = LoadTexture(renderer, "media/brick_2_broken.bmp");
        break;
      default:
        tex = LoadTexture(renderer, "media/brick_1.bmp");
        tex_broken = LoadTexture(renderer, "media/brick_1_broken.bmp");
        break;
      }
      rendering.textures[0] = Texture{
          .sheet = tex,
          .num_frames_x = 1,
          .num_frames_y = 1,
          .frame_width = 384,
          .frame_height = 128,
          .loop = true,
      };
      rendering.textures[1] = Texture{
          .sheet = tex_broken,
          .num_frames_x = 1,
          .num_frames_y = 1,
          .frame_width = 384,
          .frame_height = 128,
          .loop = true,
      };
    }
  }

  void Update(float deltaTime, InputManager *input,
              EntityManager *entitySpawner) override {
    (void)input;
    float cooldown = getComponent<float>("brokenCooldown");
    if (cooldown > 0.0f) {
      cooldown -= deltaTime;
      if (cooldown < 0.0f) {
        cooldown = 0.0f;
        SetGhostEntity(false);
        setComponent("collidedAlready", false);
      }
      setComponent("brokenCooldown", cooldown);
    }

    if (getComponent<bool>("destroyed") && entitySpawner) {
      entitySpawner->RemoveEntity(this);
    }
  }

  void OnCollision(Entity *other, CollisionData *data) override {
    if (!other || !data)
      return;

    if (other->entityType != "Ball")
      return;

    int state = getComponent<int>("state");
    float cooldown = getComponent<float>("brokenCooldown");

    if (state == 0) {
      setComponent("state", 1);
      SetTextureState(1);
      SetGhostEntity(true);
      setComponent("brokenCooldown", 0.2f);
      setComponent("collidedAlready", true);
    } else     if (cooldown == 0.0f) {
      setComponent("destroyed", true);
      SetGhostEntity(true);
      setComponent("collidedAlready", true);
    }
  }
};

class Heart : public Entity {
  inline static MemoryPool *MemPool = nullptr;

public:
  void *operator new(size_t size) {
    if (!Heart::MemPool)
      Heart::MemPool = new MemoryPool(sizeof(Heart), 16);
    int sl_id = Heart::MemPool->alloc();
    if (sl_id == -1)
      return nullptr;
    return Heart::MemPool->getPtr(sl_id);
  }

  void operator delete(void *ptr) {
    Heart::MemPool->freeSlot(Heart::MemPool->getSlot(ptr));
  }

  Heart(float x, float y, float w, float h, Timeline *tl,
        SDL_Renderer *renderer = nullptr)
      : Entity(x, y, w, h, tl) {

    entityType = "Heart";
    EnablePhysics(false);
    EnableCollision(false, false);
    setComponent("destroyed", false);

    if (renderer) {
      SDL_Texture *tex = LoadTexture(renderer, "media/heart.bmp");
      rendering.textures[0] = Texture{
          .sheet = tex,
          .num_frames_x = 1,
          .num_frames_y = 1,
          .frame_width = 32,
          .frame_height = 32,
          .loop = true,
      };
    }
  }

  void Update(float deltaTime, InputManager *input,
              EntityManager *entitySpawner) override {
    (void)deltaTime;
    (void)input;

    if (getComponent<bool>("destroyed") && entitySpawner) {
      entitySpawner->RemoveEntity(this);
    }
  }
};

inline void Ball::ResetGame(EntityManager *entitySpawner) {
  SDL_Renderer *renderer = nullptr;
  if (hasComponent("rendererPtr")) {
    renderer = reinterpret_cast<SDL_Renderer *>(
        getComponent<void *>("rendererPtr"));
  }

  // Remove all existing bricks
  std::vector<Entity *> bricksToRemove;
  for (Entity *e : entitySpawner->getEntityVectorRef()) {
    if (e && (e->entityType == "Brick_1" || e->entityType == "Brick_2")) {
      bricksToRemove.push_back(e);
    }
  }
  for (Entity *brick : bricksToRemove) {
    entitySpawner->RemoveEntity(brick);
  }

  // Create new hearts (3 hearts in top left corner)
  const float heartSize = 32.0f;
  const float heartGap = 10.0f;
  const float heartStartX = 20.0f;
  const float heartStartY = 20.0f;

  for (int i = 0; i < 3; ++i) {
    float x = heartStartX + i * (heartSize + heartGap);
    Heart *heart =
        new Heart(x, heartStartY, heartSize, heartSize, timeline, renderer);
    entitySpawner->AddEntity(heart);
  }

  const int rows = 6;
  const int cols = 8;
  const float brickWidth = 96.0f;
  const float brickHeight = 32.0f;
  const float gapX = 15.0f;
  const float gapY = 15.0f;
  float screenWidth = getComponent<float>("screenWidth");
  const float totalWidth = cols * brickWidth + (cols - 1) * gapX;
  const float startX = (screenWidth - totalWidth) * 0.5f;
  const float startY = 100.0f;

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      float x = startX + c * (brickWidth + gapX);
      float y = startY + r * (brickHeight + gapY);
      int brickType = (r + c) % 2;
      Brick *brick =
          new Brick(x, y, brickWidth, brickHeight, brickType, timeline, renderer);
      entitySpawner->AddEntity(brick);
    }
  }
}
