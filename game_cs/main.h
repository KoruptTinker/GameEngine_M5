#pragma once
#include "Core/GameEngine.h"
#include "Events/EventSystem.h"
#include "Memory/MemoryPool.h"
#include <algorithm>
#include <ctime>
#include <iostream>
#include <vector>


class Platform : public Entity {
  inline static MemoryPool *MemPool = nullptr;

  public:
    enum class PlatformType {
      STATIC,           // No movement
      MOVING_HORIZONTAL // Moves horizontally and wraps around
    };

  private:
    PlatformType platformType;
    float wrapLeftX;   // X position to wrap back to when going off left edge
    float wrapRightX;  // X position threshold when platform should wrap
    float screenWidth; // Store screen width for wrapping logic
  
  public:
    void *operator new(size_t size) {
      if (!Platform::MemPool)
        Platform::MemPool = new MemoryPool(sizeof(Platform), 128);
      int sl_id = Platform::MemPool->alloc();
      if (sl_id == -1)
        return nullptr;
      return Platform::MemPool->getPtr(sl_id);
    }

    void operator delete(void *ptr) {
      Platform::MemPool->freeSlot(Platform::MemPool->getSlot(ptr));
    }

    Platform(float x, float y, float w = 200, float h = 20, 
             PlatformType type = PlatformType::STATIC, 
             float speed = 100.0f,
             float screenW = 1000.0f,
             Timeline* timeline = nullptr)
        : Entity(x, y, w, h, timeline), platformType(type), 
          wrapLeftX(0), wrapRightX(0), screenWidth(screenW) {
      EnableCollision(false, true);
      EnablePhysics(false);

      // Set up movement parameters based on type
      if (platformType == PlatformType::STATIC) {
        SetVelocityX(0.0f);
        SetVelocityY(0.0f);
      } else if (platformType == PlatformType::MOVING_HORIZONTAL) {
        // Moving platforms move from right to left (negative velocity)
        SetVelocityX(-speed);
        SetVelocityY(0.0f);
        // When platform goes completely off left edge, wrap to right side
        wrapRightX = -w; // When right edge of platform goes off left of screen
        wrapLeftX = screenWidth; // Wrap to right side of screen
      }

      entityType = "Platform";
    }
  
    void Update(float dt, InputManager *input, EntityManager *entityManager) override {
      (void)input;
      (void)entityManager;

      // Update position based on velocity (since physics is disabled for platforms)
      if (platformType == PlatformType::MOVING_HORIZONTAL) {
        position.x += GetVelocityX() * dt;
        
        // Wrap around when platform goes completely off left edge
        if (position.x < wrapRightX) {
          position.x = wrapLeftX;
        }
      }
    }  
};


class Player : public Entity {
  inline static MemoryPool *MemPool = nullptr;

public:
  void *operator new(size_t size) {
    if (!Player::MemPool)
      Player::MemPool = new MemoryPool(sizeof(Player), 16);
    int sl_id = Player::MemPool->alloc();
    if (sl_id == -1)
      return nullptr;
    return Player::MemPool->getPtr(sl_id);
  }

  void operator delete(void *ptr) {
    Player::MemPool->freeSlot(Player::MemPool->getSlot(ptr));
  }

  Player(float x, float y, SDL_Texture *idle, SDL_Texture *runLeft,
             SDL_Texture *runRight, SDL_Texture *jumpLeft,
             SDL_Texture *jumpRight, Timeline* timeline = nullptr)
      : Entity(x, y, 100, 64, timeline) {
    EnablePhysics(true);
    EnableCollision(false, false);
    SetVelocityX(0.0f);
    SetVelocityY(0.0f);
    setComponent("grounded", false);
    setComponent("playerInputDirection", 0);
    setComponent("wasGrounded", false);
    setComponent("wasMoving", false);
    setComponent("flipAnimation", false);
    setComponent("loopAnimation", true);
    SetCurrentFrame(0);
    setComponent("lastFrameTime", (Uint32)0);
    setComponent("animationDelay", 200);
    setComponent("groundRef", (Entity*)nullptr);
    setComponent("groundVX", 0.0f);
    setComponent("IDLE_STATE", 0);
    setComponent("RUN_LEFT_STATE", 1);
    setComponent("RUN_RIGHT_STATE", 2);
    setComponent("JUMP_LEFT_STATE", 3);
    setComponent("JUMP_RIGHT_STATE", 4);

    entityType = "Player";

    // Set up texture states using the new system
    Texture idleTexture = {idle, 4, 1, 100, 64, true};
    Texture runLeftTexture = {runLeft, 6, 1, 100, 64, true};
    Texture runRightTexture = {runRight, 6, 1, 100, 64, true};
    Texture jumpLeftTexture = {jumpLeft, 6, 1, 100, 64, true};
    Texture jumpRightTexture = {jumpRight, 6, 1, 100, 64, true};

    SetTexture(getComponent<int>("IDLE_STATE"), &idleTexture);
    SetTexture(getComponent<int>("RUN_LEFT_STATE"), &runLeftTexture);
    SetTexture(getComponent<int>("RUN_RIGHT_STATE"), &runRightTexture);
    SetTexture(getComponent<int>("JUMP_LEFT_STATE"), &jumpLeftTexture);
    SetTexture(getComponent<int>("JUMP_RIGHT_STATE"), &jumpRightTexture);
  }

  void Update(float deltaTime, InputManager *input, EntityManager *entityManager) override {
    (void)entityManager;
    // Update animation
    Uint32 lastFrameTime = getComponent<Uint32>("lastFrameTime");
    int animationDelay = getComponent<int>("animationDelay");
    lastFrameTime += (Uint32)(deltaTime * 1000); // Convert to milliseconds
    setComponent("lastFrameTime", lastFrameTime);
    if (lastFrameTime >= (Uint32)animationDelay) {
      auto it = rendering.textures.find(rendering.currentTextureState);
      if (it != rendering.textures.end()) {
        const Texture& tex = it->second;
        bool loopAnimation = getComponent<bool>("loopAnimation");
        if (getComponent<bool>("flipAnimation")) {
          rendering.currentFrame = loopAnimation ? (rendering.currentFrame - 1 + tex.num_frames_x) %
                                             tex.num_frames_x
                                       : std::max(rendering.currentFrame - 1, 0);
        } else {
          rendering.currentFrame = loopAnimation ? (rendering.currentFrame + 1) % tex.num_frames_x
                                       : std::min(rendering.currentFrame + 1,
                                                  (int)rendering.textures[rendering.currentTextureState].num_frames_x - 1);
        }
      }
      setComponent("lastFrameTime", (Uint32)0);
    }

    // Input handling is now done via OnActivity calls from GameServer
    // Animation state management based on player input direction (not velocity)
    if (getComponent<int>("playerInputDirection") < 0) {
      // Player is inputting left movement
      if (getComponent<bool>("grounded")) {
        setComponent("flipAnimation", true);
        SetTextureState(getComponent<int>("RUN_LEFT_STATE"));
        setComponent("wasMoving", true);
      }
    } else if (getComponent<int>("playerInputDirection") > 0) {
      // Player is inputting right movement
      if (getComponent<bool>("grounded")) {
        setComponent("flipAnimation", false);
        SetTextureState(getComponent<int>("RUN_RIGHT_STATE"));
        setComponent("wasMoving", true);
      }
    } else {
      // Player is idle (no input)
      setComponent("wasMoving", false);
      if (getComponent<bool>("grounded")) {
        SetTextureState(getComponent<int>("IDLE_STATE"));
      }
    }

    // Handle jump animation state
    if (!getComponent<bool>("grounded") && getComponent<bool>("wasGrounded")) {
      setComponent("wasGrounded", false);
      if (getComponent<int>("playerInputDirection") < 0) {
        setComponent("flipAnimation", true);
        SetTextureState(getComponent<int>("JUMP_LEFT_STATE"));
      } else {
        setComponent("flipAnimation", false);
        SetTextureState(getComponent<int>("JUMP_RIGHT_STATE"));
      }
    }

    // Bounce off screen edges
    if (position.x <= 0) {
      position.x = 0;
    } else if (position.x + dimensions.x >= 1000) {
      position.x = 1000 - dimensions.x;
    }

    // Respawn on fall (original logic)
    const float screenHeight = 1000.0f;
    if (position.y > screenHeight) {
      // Respawn at original spawn position
      position.x = 100.0f;
      position.y = 100.0f;
      SetVelocityX(0.0f);
      SetVelocityY(0.0f);
      setComponent("grounded", false);
      setComponent("groundRef", (Entity*)nullptr);
      setComponent("groundVX", 0.0f);
      std::cout << "Player fell! Respawning at original position." << std::endl;
    }

    // Maintain horizontal velocity based on input direction when grounded
    // This ensures movement continues even if velocity is reset elsewhere
    constexpr float runSpeed = 200.0f;
    int inputDir = getComponent<int>("playerInputDirection");
    if (inputDir < 0) {
      // Moving left
      SetVelocityX(-runSpeed);
    } else if (inputDir > 0) {
      // Moving right
      SetVelocityX(runSpeed);
    } else if (getComponent<bool>("grounded")) {
      // Idle and grounded - inherit platform velocity
      Entity* groundRef = getComponent<Entity*>("groundRef");
      float carrierVX = (groundRef != nullptr) ? groundRef->GetVelocityX() : 0.0f;
      SetVelocityX(carrierVX);
    }

    // Clear ground reference if not grounded
    if (!getComponent<bool>("grounded")) {
      setComponent("groundRef", (Entity*)nullptr);
      setComponent("groundVX", 0.0f);
    }
  }

  void OnActivity(const std::string& actionName) override {
    // speeds
    constexpr float runSpeed = 200.0f;
    
    if (actionName == "MOVE_LEFT") {
      // Move left at constant speed, ignoring platform motion
      SetVelocityX(-runSpeed);
      setComponent("playerInputDirection", -1);
    } else if (actionName == "MOVE_RIGHT") {
      // Move right at constant speed, ignoring platform motion
      SetVelocityX(runSpeed);
      setComponent("playerInputDirection", 1);
    } else if (actionName == "JUMP" && getComponent<bool>("grounded")) {
      SetVelocityY(-1500.0f);
      setComponent("grounded", false);
    } else if (actionName == "IDLE") {
      // Stop horizontal movement, inherit platform velocity when grounded
      float carrierVX = (getComponent<bool>("grounded") && getComponent<Entity*>("groundRef")) ? getComponent<Entity*>("groundRef")->GetVelocityX() : 0.0f;
      SetVelocityX(carrierVX);
      setComponent("playerInputDirection", 0);
    } else {
      // Default case - also treat as IDLE
      float carrierVX = (getComponent<bool>("grounded") && getComponent<Entity*>("groundRef")) ? getComponent<Entity*>("groundRef")->GetVelocityX() : 0.0f;
      SetVelocityX(carrierVX);
      setComponent("playerInputDirection", 0);
    }
  }

  void OnCollision(Entity *other, CollisionData *collData) override {
    if (dynamic_cast<Platform*>(other) && collData->normal.y == -1.0f && collData->normal.x == 0.0f) {
      if (!getComponent<bool>("wasGrounded") || !getComponent<bool>("wasMoving")) {
        SetTextureState(getComponent<int>("IDLE_STATE"));
        setComponent("wasGrounded", true);
        setComponent("wasMoving", false);
      }
      setComponent("grounded", true);
      SetVelocityY(0.0f);
      setComponent("groundRef", other);
    }
    // Removed side collision velocity stopping to allow movement
  }

  // Get current frame for rendering
  bool GetSourceRect(SDL_FRect &out) const override {
    out = SampleTextureAt(rendering.currentFrame, 0);
    return true;
  }
};
