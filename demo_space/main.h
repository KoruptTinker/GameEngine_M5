#pragma once

#include "Core/GameEngine.h"
#include "Core/Render.h"
#include <cstdlib>

class Bullet : public Entity {

  inline static MemoryPool *MemPool = nullptr;
  inline static std::vector<Bullet *> pendingDeletions;

public:
  void *operator new(size_t size) {
    if (!Bullet::MemPool)
      Bullet::MemPool = new MemoryPool(sizeof(Bullet), 128);
    int sl_id = Bullet::MemPool->alloc();
    if (sl_id == -1)
      return nullptr;
    return Bullet::MemPool->getPtr(sl_id);
  }

  void operator delete(void *ptr) {
    Bullet::MemPool->freeSlot(Bullet::MemPool->getSlot(ptr));
  }

  Bullet(float x, float y, float w, float h, Entity *parent, Timeline *tl,
         SDL_Renderer *renderer = nullptr)
      : Entity(x, y, w, h, tl) {
    EnablePhysics(false);
    // Enable collision as ghost entity (true = ghost, false = not kinematic)
    EnableCollision(true, false);

    entityType = "Bullet";
    setComponent("parent", parent);
    setComponent("markedForDeletion", false);
    setComponent("isPlayerBullet",
                 false); // Default to invader bullet, will be set explicitly
    setComponent("screenHeight", 1000.0f); // Default screen height

    if (renderer) {
      SDL_Texture *tex = LoadTexture(renderer, "media/bullet.bmp");
      if (tex) {
        rendering.textures[0] = Texture{
            .sheet = tex,
            .num_frames_x = 1,
            .num_frames_y = 1,
            .frame_width = 10,
            .frame_height = 10,
            .loop = true,
        };
      }
    }
  }

  void Update(float deltaTime, InputManager *input,
              EntityManager *entitySpawner) override {
    // Check if marked for deletion (from collision or going off screen)
    bool shouldDelete = false;
    if (hasComponent("markedForDeletion")) {
      shouldDelete = getComponent<bool>("markedForDeletion");
    }

    // Get bullet type and screen height
    bool isPlayerBullet = false;
    if (hasComponent("isPlayerBullet")) {
      isPlayerBullet = getComponent<bool>("isPlayerBullet");
    }

    float screenHeight = 1000.0f;
    if (hasComponent("screenHeight")) {
      screenHeight = getComponent<float>("screenHeight");
    }

    // Delete player bullets when they go above y = 0 (top of screen)
    if (isPlayerBullet && position.y < 0.0f) {
      shouldDelete = true;
    }

    // Delete invader bullets when they go below screen (y > screenHeight)
    if (!isPlayerBullet && position.y > screenHeight) {
      shouldDelete = true;
    }

    if (shouldDelete && entitySpawner) {
      // Disable collision first (before removing from manager) to prevent
      // further collision events
      collisionEnabled = false;
      rendering.isVisible = false; // Hide immediately
      physicsEnabled = false;      // Disable physics

      // Mark for deferred deletion to avoid mutex issues during update loop
      entitySpawner->RemoveEntity(this);
      pendingDeletions.push_back(this);
      return; // Exit early - bullet is marked for deletion
    }
  }

  void OnCollision(Entity *other, CollisionData *data) override {
    // Early return if invalid
    if (!other || !collisionEnabled) {
      return;
    }

    // Check if other entity is valid and not being deleted (check before
    // accessing anything else)
    if (!other->collisionEnabled) {
      return; // Other entity is being deleted
    }

    // Check if already marked for deletion
    if (hasComponent("markedForDeletion") &&
        getComponent<bool>("markedForDeletion")) {
      return; // Already marked for deletion
    }

    // Get bullet type
    bool isPlayerBullet = false;
    if (hasComponent("isPlayerBullet")) {
      isPlayerBullet = getComponent<bool>("isPlayerBullet");
    }

    // Player bullets collide with invaders
    if (isPlayerBullet &&
        (other->entityType == "Invader_0" || other->entityType == "Invader_1" ||
         other->entityType == "Invader_2")) {
      // Set flag to mark for deletion (will be handled in Update)
      setComponent("markedForDeletion", true);
    }

    // Invader bullets collide with player
    if (!isPlayerBullet && other->entityType == "Player") {
      // Mark bullet for deletion
      setComponent("markedForDeletion", true);
      // Mark player for death/respawn
      other->setComponent("markedForDeath", true);
    }
  }

  // Static method to process pending deletions (call after all
  // updates/physics/collisions)
  static void ProcessPendingDeletions() {
    for (Bullet *bullet : pendingDeletions) {
      delete bullet;
    }
    pendingDeletions.clear();
  }
};

class Invader : public Entity {

  inline static MemoryPool *MemPool = nullptr;
  inline static std::vector<Invader *> pendingDeletions;

  // Shared state for all invaders to move together
  inline static float moveDirection = 1.0f; // 1.0 = right, -1.0 = left
  inline static bool shouldMoveDown = false;
  inline static bool directionReversedThisFrame =
      false; // Prevent multiple reversals per frame
  inline static float invaderSpeed = 50.0f; // Horizontal speed
  inline static float downStep =
      40.0f; // How much to move down when hitting edge (one step)
  inline static float resetTimer = 0.0f; // Shared timer for resetting flags
  inline static int moveDownFrameCounter = 0; // Track frame when move down was triggered
  inline static int lastMoveDownFrame = -1; // Track the last frame counter when we reset the flag
  inline static float shootTimer = 0.0f; // Shared timer for invader shooting

public:
  void *operator new(size_t size) {
    if (!Invader::MemPool)
      Invader::MemPool = new MemoryPool(sizeof(Invader), 128);
    int sl_id = Invader::MemPool->alloc();
    if (sl_id == -1)
      return nullptr;
    return Invader::MemPool->getPtr(sl_id);
  }

  void operator delete(void *ptr) {
    Invader::MemPool->freeSlot(Invader::MemPool->getSlot(ptr));
  }

  Invader(float x, float y, float w, float h, int invader_type, Timeline *tl,
          SDL_Renderer *renderer = nullptr)
      : Entity(x, y, w, h, tl) {
    EnablePhysics(false);
    // Enable collision as ghost entity (true = ghost, false = not kinematic)
    EnableCollision(true, false);

    setComponent("markedForDeletion", false);
    setComponent("screenWidth", 1000.0f);
    setComponent("screenHeight", 1000.0f);
    setComponent("shootCooldown", 0.0f); // Individual invader shoot cooldown
    setComponent("rendererPtr", (void *)renderer);
    setComponent("hasMovedDown", false); // Track if this invader has moved down for current edge hit

    // Set entityType regardless of renderer
    switch (invader_type) {
    case 0:
      entityType = "Invader_0";
      break;
    case 1:
      entityType = "Invader_1";
      break;
    case 2:
      entityType = "Invader_2";
      break;
    default:
      entityType = "Invader_1";
      break;
    }

    if (renderer) {
      SDL_Texture *tex = nullptr;
      switch (invader_type) {
      case 0:
        tex = LoadTexture(renderer, "media/red.bmp");
        break;
      case 1:
        tex = LoadTexture(renderer, "media/yellow.bmp");
        break;
      case 2:
        tex = LoadTexture(renderer, "media/green.bmp");
        break;
      default:
        tex = LoadTexture(renderer, "media/yellow.bmp");
        break;
      }
      // Only set texture if loading succeeded
      if (tex) {
        rendering.textures[0] = Texture{
            .sheet = tex,
            .num_frames_x = 1,
            .num_frames_y = 1,
            .frame_width = 40,
            .frame_height = 32,
            .loop = true,
        };
      }
    }
  }

  void Update(float deltaTime, InputManager *input,
              EntityManager *entitySpawner) override {
    // Check if marked for deletion (from collision)
    bool shouldDelete = false;
    if (hasComponent("markedForDeletion")) {
      shouldDelete = getComponent<bool>("markedForDeletion");
    }

    if (shouldDelete && entitySpawner) {
      // Disable collision first (before removing from manager) to prevent
      // further collision events
      collisionEnabled = false;
      rendering.isVisible = false; // Hide immediately
      physicsEnabled = false;      // Disable physics

      // Mark for deferred deletion to avoid mutex issues during update loop
      entitySpawner->RemoveEntity(this);
      pendingDeletions.push_back(this);
      return; // Exit early - invader is marked for deletion
    }

    // Get screen boundaries
    float screenWidth = 1000.0f;
    float screenHeight = 1000.0f;
    if (hasComponent("screenWidth")) {
      screenWidth = getComponent<float>("screenWidth");
    }
    if (hasComponent("screenHeight")) {
      screenHeight = getComponent<float>("screenHeight");
    }

    // Move horizontally first
    float horizontalMovement = moveDirection * invaderSpeed * deltaTime;
    position.x += horizontalMovement;

    // Find leftmost and rightmost invaders to check boundaries after moving
    // This ensures all invaders reverse together when the group hits an edge
    if (entitySpawner && !directionReversedThisFrame) {
      float leftmostX = screenWidth;
      float rightmostX = 0.0f;
      bool foundInvader = false;

      for (Entity *e : entitySpawner->getEntityVectorRef()) {
        if (e &&
            (e->entityType == "Invader_0" || e->entityType == "Invader_1" ||
             e->entityType == "Invader_2")) {
          foundInvader = true;
          if (e->position.x < leftmostX) {
            leftmostX = e->position.x;
          }
          if (e->position.x + e->dimensions.x > rightmostX) {
            rightmostX = e->position.x + e->dimensions.x;
          }
        }
      }

      // Check if group hit boundary after moving
      if (foundInvader) {
        bool hitLeft = (leftmostX <= 0.0f && moveDirection < 0.0f);
        bool hitRight = (rightmostX >= screenWidth && moveDirection > 0.0f);

        if (hitLeft || hitRight) {
          moveDirection = -moveDirection;
          shouldMoveDown = true;
          moveDownFrameCounter++; // Increment to signal new move down cycle
          directionReversedThisFrame = true;
          // Reverse the movement we just made
          position.x -= horizontalMovement;
          
          // Reset all invaders' hasMovedDown flag by iterating through them
          if (entitySpawner) {
            for (Entity *e : entitySpawner->getEntityVectorRef()) {
              if (e && (e->entityType == "Invader_0" || e->entityType == "Invader_1" ||
                       e->entityType == "Invader_2")) {
                e->setComponent("hasMovedDown", false);
              }
            }
          }
        }
      }
    }

    // Move down if needed (all invaders move down together)
    bool hasMovedDown = false;
    if (hasComponent("hasMovedDown")) {
      hasMovedDown = getComponent<bool>("hasMovedDown");
    }
    
    if (shouldMoveDown && !hasMovedDown) {
      position.y += downStep;
      setComponent("hasMovedDown", true); // Mark that this invader has moved down
    }
    
    // Reset flag after all invaders have had a chance to move down
    // Check if the frame counter has changed since we last reset, meaning all invaders
    // have had at least one update cycle to process the move down
    if (shouldMoveDown && moveDownFrameCounter != lastMoveDownFrame) {
      // Check if all invaders have moved down by counting how many still need to
      if (entitySpawner) {
        int totalInvaders = 0;
        int movedDown = 0;
        for (Entity *e : entitySpawner->getEntityVectorRef()) {
          if (e && (e->entityType == "Invader_0" || e->entityType == "Invader_1" ||
                   e->entityType == "Invader_2")) {
            totalInvaders++;
            bool invaderHasMoved = false;
            if (e->hasComponent("hasMovedDown")) {
              invaderHasMoved = e->getComponent<bool>("hasMovedDown");
            }
            if (invaderHasMoved) {
              movedDown++;
            }
          }
        }
        // If all invaders have moved down, or if we've waited long enough, reset the flag
        if (totalInvaders > 0 && movedDown == totalInvaders) {
          shouldMoveDown = false;
          lastMoveDownFrame = moveDownFrameCounter;
        }
      } else {
        // Fallback: reset after frame counter changes (all invaders updated at least once)
        lastMoveDownFrame = moveDownFrameCounter;
      }
    }

    // Clamp horizontal position to screen boundaries
    if (position.x < 0.0f) {
      position.x = 0.0f;
    }
    if (position.x + dimensions.x > screenWidth) {
      position.x = screenWidth - dimensions.x;
    }

    // Keep vertical position within bounds
    if (position.y < 0.0f) {
      position.y = 0.0f;
    }
    if (position.y + dimensions.y > screenHeight) {
      position.y = screenHeight - dimensions.y;
    }

    // Reset direction reversal flag after a short delay
    resetTimer += deltaTime;
    if (resetTimer > 0.05f) { // Reset after 50ms
      directionReversedThisFrame = false;
      resetTimer = 0.0f;
    }

    // Random shooting logic for invaders
    float shootCooldown = 0.0f;
    if (hasComponent("shootCooldown")) {
      shootCooldown = getComponent<float>("shootCooldown");
    }

    if (shootCooldown > 0.0f) {
      shootCooldown -= deltaTime;
      if (shootCooldown < 0.0f)
        shootCooldown = 0.0f;
      setComponent("shootCooldown", shootCooldown);
    }

    // Update shared shoot timer for random shooting
    shootTimer += deltaTime;
    constexpr float shootInterval =
        0.5f; // Try shooting every 0.5 seconds on average
    if (shootTimer >= shootInterval && shootCooldown <= 0.0f && entitySpawner) {
      // Random chance to shoot (each invader has independent chance)
      float randomValue =
          static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      constexpr float shootProbability = 0.05f; // 5% chance per interval

      if (randomValue < shootProbability) {
        // Shoot a bullet downward
        SDL_Renderer *renderer = nullptr;
        if (hasComponent("rendererPtr")) {
          renderer = reinterpret_cast<SDL_Renderer *>(
              getComponent<void *>("rendererPtr"));
        }

        if (renderer) {
          const float bulletWidth = 10.0f;
          const float bulletHeight = 10.0f;
          const float bulletX =
              position.x + (dimensions.x - bulletWidth) * 0.5f;
          const float bulletY = position.y + dimensions.y; // Below the invader

          Bullet *bullet = new Bullet(bulletX, bulletY, bulletWidth,
                                      bulletHeight, this, timeline, renderer);

          if (bullet != nullptr) {
            // Mark as invader bullet (not player bullet)
            bullet->setComponent("isPlayerBullet", false);
            bullet->setComponent("screenHeight", screenHeight);

            // Enable physics for movement
            bullet->EnablePhysics(false);

            // Set downward velocity for invader bullet (positive Y = downward)
            constexpr float bulletSpeed = 1000.0f;
            bullet->SetVelocityY(bulletSpeed);
            bullet->SetVelocityX(0.0f);

            // Add bullet to entity manager
            entitySpawner->AddEntity(bullet);

            // Set individual cooldown to prevent spam
            setComponent("shootCooldown",
                         1.1f); // 0.8 second cooldown per invader
          }
        }
      }

      // Reset shared timer
      shootTimer = 0.0f;
    }
  }

  void OnCollision(Entity *other, CollisionData *data) override {
    // Early return if invalid
    if (!other || !collisionEnabled) {
      return;
    }

    // Check if other entity is valid and not being deleted (check before
    // accessing anything else)
    if (!other->collisionEnabled) {
      return; // Other entity is being deleted
    }

    // Check if already marked for deletion
    if (hasComponent("markedForDeletion") &&
        getComponent<bool>("markedForDeletion")) {
      return; // Already marked for deletion
    }

    // Check if colliding with a player bullet (invader bullets move down, so
    // they won't hit invaders)
    if (other->entityType == "Bullet") {
      // Only player bullets can hit invaders (they move upward)
      bool isPlayerBullet = false;
      if (other->hasComponent("isPlayerBullet")) {
        isPlayerBullet = other->getComponent<bool>("isPlayerBullet");
      }
      if (isPlayerBullet) {
        // Set flag to mark for deletion (will be handled in Update)
        setComponent("markedForDeletion", true);
      }
    }
  }

  // Static method to process pending deletions (call after all
  // updates/physics/collisions)
  static void ProcessPendingDeletions() {
    for (Invader *invader : pendingDeletions) {
      delete invader;
    }
    pendingDeletions.clear();
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

  Player(float x, float y, float w, float h, Timeline *tl,
         SDL_Renderer *renderer = nullptr)
      : Entity(x, y, w, h, tl) {
    EnablePhysics(false);
    EnableCollision(
        true,
        false); // Enable collision so player can be hit by invader bullets

    entityType = "Player";

    if (renderer) {
      SDL_Texture *tex = LoadTexture(renderer, "media/player.bmp");
      if (tex) {
        rendering.textures[0] = Texture{
            .sheet = tex,
            .num_frames_x = 1,
            .num_frames_y = 1,
            .frame_width = 60,
            .frame_height = 30,
            .loop = true,
        };
        SetTextureState(0);
      }
      setComponent("rendererPtr", (void *)renderer);
    }
    setComponent("dashLeftActive", false);
    setComponent("dashRightActive", false);
    setComponent("shouldShoot", false);
    setComponent("shootCooldown", 0.0f); // Cooldown timer in seconds
    setComponent("markedForDeath", false);
    setComponent("initialX", x); // Store initial position for respawn
    setComponent("initialY", y);
    setComponent("initialWidth", w);
    setComponent("initialHeight", h);
  }

  void Update(float deltaTime, InputManager *input,
              EntityManager *entitySpawner) override {
    // Process pending bullet deletions from previous frame
    // This must happen at a safe time (before any bullet updates)
    Bullet::ProcessPendingDeletions();

    // Process pending invader deletions from previous frame
    Invader::ProcessPendingDeletions();

    // Check if player is marked for death/respawn
    bool markedForDeath = false;
    if (hasComponent("markedForDeath")) {
      markedForDeath = getComponent<bool>("markedForDeath");
    }

    if (markedForDeath) {
      // Respawn player at initial position
      float initialX = getComponent<float>("initialX");
      float initialY = getComponent<float>("initialY");
      float initialWidth = getComponent<float>("initialWidth");
      float initialHeight = getComponent<float>("initialHeight");

      position.x = initialX;
      position.y = initialY;
      dimensions.x = initialWidth;
      dimensions.y = initialHeight;

      // Reset velocity
      SetVelocityX(0.0f);
      SetVelocityY(0.0f);

      // Reset death flag
      setComponent("markedForDeath", false);

      // Make visible again
      rendering.isVisible = true;
      collisionEnabled = true;
    }

    // Update shoot cooldown
    float cooldown = getComponent<float>("shootCooldown");
    if (cooldown > 0.0f) {
      cooldown -= deltaTime;
      if (cooldown < 0.0f)
        cooldown = 0.0f;
      setComponent("shootCooldown", cooldown);
    }

    setComponent("dashLeftActive", false);
    setComponent("dashRightActive", false);

    // Check if we should shoot and spawn bullet (only if cooldown is ready)
    // Get the updated cooldown value
    float currentCooldown = getComponent<float>("shootCooldown");
    if (hasComponent("shouldShoot") && getComponent<bool>("shouldShoot") &&
        currentCooldown <= 0.0f) {
      SDL_Renderer *renderer = nullptr;

      if (hasComponent("rendererPtr")) {
        renderer = reinterpret_cast<SDL_Renderer *>(
            getComponent<void *>("rendererPtr"));
      }

      if (entitySpawner) {
        // Create bullet at player's position, centered horizontally, above the
        // player
        const float bulletWidth = 10.0f;
        const float bulletHeight = 10.0f;
        const float bulletX = position.x + (dimensions.x - bulletWidth) * 0.5f;
        const float bulletY = position.y - bulletHeight; // Above the player

        Bullet *bullet = new Bullet(bulletX, bulletY, bulletWidth, bulletHeight,
                                    this, timeline, renderer);

        // Check if bullet creation succeeded (memory pool might be full)
        if (bullet != nullptr) {
          // Mark as player bullet
          bullet->setComponent("isPlayerBullet", true);

          // Get screen height for bullet
          float screenHeight = 1000.0f;
          if (hasComponent("screenHeight")) {
            screenHeight = getComponent<float>("screenHeight");
          }
          bullet->setComponent("screenHeight", screenHeight);

          // Enable physics FIRST so the bullet can move (false = no gravity)
          bullet->EnablePhysics(false);

          // Set upward velocity for the bullet (must be after enabling physics)
          constexpr float bulletSpeed = -1000.0f; // Negative Y = upward
          bullet->SetVelocityY(bulletSpeed);
          bullet->SetVelocityX(0.0f);

          // Add bullet to entity manager
          entitySpawner->AddEntity(bullet);

          // Set cooldown after shooting (0.3 seconds = 300ms)
          constexpr float shootCooldownTime = 0.3f;
          setComponent("shootCooldown", shootCooldownTime);
        }
        // If bullet creation failed (memory pool full), we still reset the flag
        // and continue with the rest of the update
      }

      // Reset the flag after spawning to prevent multiple bullets
      setComponent("shouldShoot", false);
    }

    // Pause functionality
    static bool pKeyWasPressed = false;
    bool pKeyIsPressed = input->IsKeyPressed(SDL_SCANCODE_P);

    if (pKeyIsPressed && !pKeyWasPressed) {
      // Key was just pressed (not held)
      if (timeline->getState() == Timeline::State::PAUSE) {
        timeline->setState(Timeline::State::RUN);
      } else {
        timeline->setState(Timeline::State::PAUSE);
      }
    }
    pKeyWasPressed = pKeyIsPressed;

    // Clamp bumper so it doesn't run off the screen edges
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
      if (!getComponent<bool>("dashLeftActive")) {
        SetVelocityX(-moveSpeed);
      }
    } else if (actionName == "MOVE_RIGHT") {
      if (!getComponent<bool>("dashRightActive")) {
        SetVelocityX(moveSpeed);
      }
    } else if (actionName == "DASH_LEFT") {
      SetVelocityX(-dashSpeed);
      setComponent("dashLeftActive", true);
    } else if (actionName == "DASH_RIGHT") {
      SetVelocityX(dashSpeed);
      setComponent("dashRightActive", true);
    } else if (actionName == "SHOOT") {
      // Set flag to spawn bullet in Update function (only if cooldown is ready
      // and not already set)
      float cooldown = getComponent<float>("shootCooldown");
      if (cooldown <= 0.0f && (!hasComponent("shouldShoot") ||
                               !getComponent<bool>("shouldShoot"))) {
        setComponent("shouldShoot", true);
      }
    } else {
      SetVelocityX(0.0f);
    }
  }
};
