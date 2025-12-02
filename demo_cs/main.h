#pragma once
#include <iostream>
#include <cmath>

#include "Core/GameEngine.h"
#include "Core/Render.h"
#include "Entities/Entity.h"
#include "Math/vec2.h"

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
    // Default screen width; can be overridden via component if needed
    setComponent("screenWidth", 1800.0f);
    EnablePhysics(false);
    EnableCollision(false, false);
    setComponent("lastFrameTime", Uint32(0));
    setComponent("animationDelay", 100);
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
      (void)entitySpawner;

      Uint32 lastFrameTime = getComponent<Uint32>("lastFrameTime");
      int animationDelay = getComponent<int>("animationDelay");
      lastFrameTime += (Uint32)(deltaTime * 1000);  // Convert to milliseconds
      if (lastFrameTime >= (Uint32)animationDelay) {
        rendering.currentFrame =
            (rendering.currentFrame + 1) %
            rendering.textures[rendering.currentTextureState].num_frames_x;
        lastFrameTime = 0;
      }
      setComponent("lastFrameTime", lastFrameTime);

      setComponent("grounded", false);

      if (position.x <= 0) {
        position.x = 0;
      }

      if (position.y > 1080) {
        position.x = 100;
        position.y = 100;
        SetVelocityY(0.0f);
        setComponent("grounded", false);
        setComponent("groundRef", static_cast<Entity*>(nullptr));
      }

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

      static bool iKeyWasPressed = false;
      static bool oKeyWasPressed = false;
      static bool uKeyWasPressed = false;

      bool iKeyIsPressed = input->IsKeyPressed(SDL_SCANCODE_I);
      bool oKeyIsPressed = input->IsKeyPressed(SDL_SCANCODE_O);
      bool uKeyIsPressed = input->IsKeyPressed(SDL_SCANCODE_U);

      if (iKeyIsPressed && !iKeyWasPressed) {
        timeline->setScale(timeline->getScale() - 0.5f);
      }
      if (oKeyIsPressed && !oKeyWasPressed) {
        timeline->setScale(timeline->getScale() + 0.5f);
      }
      if (uKeyIsPressed && !uKeyWasPressed) {
        timeline->setScale(0.5f);
      }

      iKeyWasPressed = iKeyIsPressed;
      oKeyWasPressed = oKeyIsPressed;
      uKeyWasPressed = uKeyIsPressed;
  }

  void OnActivity(const std::string &actionName) override {
    // speeds
    constexpr float runSpeed = 200.0f;
    constexpr float dashSpeed = 900.0f;
    
    // Get ground reference and grounded state
    Entity* groundRef = getComponent<Entity*>("groundRef");
    bool grounded = getComponent<bool>("grounded");
    SDL_Log("OnActivity: actionName: %s", actionName.c_str());
    if (actionName == "MOVE_LEFT") {
      // Move left at constant speed, ignoring platform motion
      SetVelocityX(-moveSpeed);
    } else if (actionName == "MOVE_RIGHT") {
      // Move right at constant speed, ignoring platform motion
      SetVelocityX(runSpeed);
      setComponent("playerInputDirection", 1);
    } else if (actionName == "DASH_LEFT") {
      // Dash left (chord: Shift + A)
      SetVelocityX(-dashSpeed);
      setComponent("playerInputDirection", -1);
    } else if (actionName == "DASH_RIGHT") {
      // Dash right (chord: Shift + D)
      SetVelocityX(dashSpeed);
      setComponent("playerInputDirection", 1);
    } else if (actionName == "JUMP") {
      // Only jump if grounded, but don't reset horizontal velocity if not grounded
      if (grounded) {
        SetVelocityY(-1500.0f);
        setComponent("grounded", false);
        setComponent("wasGrounded", false);
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

  void Update(float deltaTime, InputManager *input,
              EntityManager *entitySpawner) override {
    (void)deltaTime;
    (void)input;
    
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

    // If the ball is attached to the bumper, keep it riding on top of the bumper
    // and don't apply any wall/boundary logic yet.
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

    // If the ball hits the bottom of the screen, reset it to ride on the bumper
    // and wait for a new launch.
    if (hitBottomBoundary) {
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
    if (!other || !data) return;

    // Handle bumper collision with angle-based bounce
    if (other->entityType == "PlayerBumper") {
      // Calculate the offset: distance between ball's center and bumper's center
      float ballCenterX = position.x + dimensions.x * 0.5f;
      float ballCenterY = position.y + dimensions.y * 0.5f;
      float bumperCenterX = other->position.x + other->dimensions.x * 0.5f;
      float bumperCenterY = other->position.y + other->dimensions.y * 0.5f;
      
      float offset = ballCenterX - bumperCenterX;
      
      float halfBumperWidth = other->dimensions.x * 0.5f;
      float normalizedOffset = offset / halfBumperWidth;
      
      if (normalizedOffset < -1.0f) normalizedOffset = -1.0f;
      if (normalizedOffset > 1.0f) normalizedOffset = 1.0f;
      
      constexpr float maxBounceAngle = 75.0f;
      float bounceAngle = normalizedOffset * maxBounceAngle;
      
      float angleRad = bounceAngle * M_PI / 180.0f;
      
      float currentVelX = GetVelocityX();
      float currentVelY = GetVelocityY();
      float currentSpeed = sqrtf(currentVelX * currentVelX + currentVelY * currentVelY);
      
      if (currentSpeed < 1.0f) {
        currentSpeed = 300.0f; 
      }
      float newVelX = currentSpeed * sinf(angleRad);
      float newVelY = -currentSpeed * cosf(angleRad); 
      
      SetVelocityX(newVelX);
      SetVelocityY(newVelY);
    } else if(other->entityType == "Brick_1" || other->entityType == "Brick_2") {
      if(other->getComponent<bool>("collidedAlready")) {
        return;
      }
      if(data->normal.y != 0) {
        SetVelocityY(-GetVelocityY());
      } else {
        SetVelocityX(-GetVelocityX());
      }
      other->setComponent("collidedAlready", true);
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
    // 0 = intact, 1 = broken
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
    // Decrease cooldown timer if active
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
    if (!other || !data) return;

    if (other->entityType != "Ball") return;

    int state = getComponent<int>("state");
    float cooldown = getComponent<float>("brokenCooldown");

    if (state == 0) {
      setComponent("state", 1);
      SetTextureState(1);
      SetGhostEntity(true);
      setComponent("brokenCooldown", 0.2f); 
      setComponent("collidedAlready", true);
    } else if (cooldown == 0.0f) {
      setComponent("destroyed", true);
      SetGhostEntity(true);
      setComponent("collidedAlready", true);
    }
  }
};
