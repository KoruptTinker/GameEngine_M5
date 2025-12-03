#pragma once

#include "Core/GameEngine.h"
#include "Core/Render.h"
#include <cstdlib>

// Forward declarations
class Heart;
class Invader;

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
    EnableCollision(true, false);

    entityType = "Bullet";
    setComponent("parent", parent);
    setComponent("markedForDeletion", false);
    setComponent("isPlayerBullet",
                 false); 
    setComponent("screenHeight", 1000.0f); 

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
    bool shouldDelete = false;
    if (hasComponent("markedForDeletion")) {
      shouldDelete = getComponent<bool>("markedForDeletion");
    }

    bool isPlayerBullet = false;
    if (hasComponent("isPlayerBullet")) {
      isPlayerBullet = getComponent<bool>("isPlayerBullet");
    }

    float screenHeight = 1000.0f;
    if (hasComponent("screenHeight")) {
      screenHeight = getComponent<float>("screenHeight");
    }

  
    if (isPlayerBullet && position.y < 0.0f) {
      shouldDelete = true;
    }

    if (!isPlayerBullet && position.y > screenHeight) {
      shouldDelete = true;
    }

    if (shouldDelete && entitySpawner) {

      collisionEnabled = false;
      rendering.isVisible = false;
      physicsEnabled = false;    

      entitySpawner->RemoveEntity(this);
      pendingDeletions.push_back(this);
      return; 
    }
  }

  void OnCollision(Entity *other, CollisionData *data) override {
    if (!other || !collisionEnabled) {
      return;
    }


    if (!other->collisionEnabled) {
      return;
    }

    if (hasComponent("markedForDeletion") &&
        getComponent<bool>("markedForDeletion")) {
      return; 
    }

    // Get bullet type
    bool isPlayerBullet = false;
    if (hasComponent("isPlayerBullet")) {
      isPlayerBullet = getComponent<bool>("isPlayerBullet");
    }


    if (isPlayerBullet &&
        (other->entityType == "Invader_0" || other->entityType == "Invader_1" ||
         other->entityType == "Invader_2")) {
      setComponent("markedForDeletion", true);
    }

    if (!isPlayerBullet && other->entityType == "Player") {
      setComponent("markedForDeletion", true);
      other->setComponent("markedForDeath", true);
    }
  }

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

  inline static float moveDirection = 1.0f; 
  inline static bool shouldMoveDown = false;
  inline static bool directionReversedThisFrame =
      false; 
  inline static float invaderSpeed = 50.0f; 
  inline static float downStep =
      40.0f; 
  inline static float resetTimer = 0.0f; 
  inline static int moveDownFrameCounter = 0; 
  inline static int lastMoveDownFrame = -1; 
  inline static float shootTimer = 0.0f; 

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
    EnableCollision(true, false);

    setComponent("markedForDeletion", false);
    setComponent("screenWidth", 1000.0f);
    setComponent("screenHeight", 1000.0f);
    setComponent("shootCooldown", 0.0f); 
    setComponent("rendererPtr", (void *)renderer);
    setComponent("hasMovedDown", false); 

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
    bool shouldDelete = false;
    if (hasComponent("markedForDeletion")) {
      shouldDelete = getComponent<bool>("markedForDeletion");
    }

    if (shouldDelete && entitySpawner) {
      collisionEnabled = false;
      rendering.isVisible = false; 
      physicsEnabled = false;     

      entitySpawner->RemoveEntity(this);
      pendingDeletions.push_back(this);
      return; 
    }

    float screenWidth = 1000.0f;
    float screenHeight = 1000.0f;
    if (hasComponent("screenWidth")) {
      screenWidth = getComponent<float>("screenWidth");
    }
    if (hasComponent("screenHeight")) {
      screenHeight = getComponent<float>("screenHeight");
    }

    float horizontalMovement = moveDirection * invaderSpeed * deltaTime;
    position.x += horizontalMovement;


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

      if (foundInvader) {
        bool hitLeft = (leftmostX <= 0.0f && moveDirection < 0.0f);
        bool hitRight = (rightmostX >= screenWidth && moveDirection > 0.0f);

        if (hitLeft || hitRight) {
          moveDirection = -moveDirection;
          shouldMoveDown = true;
          moveDownFrameCounter++; 
          directionReversedThisFrame = true;
          position.x -= horizontalMovement;
          
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

    bool hasMovedDown = false;
    if (hasComponent("hasMovedDown")) {
      hasMovedDown = getComponent<bool>("hasMovedDown");
    }
    
    if (shouldMoveDown && !hasMovedDown) {
      position.y += downStep;
      setComponent("hasMovedDown", true); 
    }
    

    if (shouldMoveDown && moveDownFrameCounter != lastMoveDownFrame) {
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
        if (totalInvaders > 0 && movedDown == totalInvaders) {
          shouldMoveDown = false;
          lastMoveDownFrame = moveDownFrameCounter;
        }
      } else {
        lastMoveDownFrame = moveDownFrameCounter;
      }
    }

    if (position.x < 0.0f) {
      position.x = 0.0f;
    }
    if (position.x + dimensions.x > screenWidth) {
      position.x = screenWidth - dimensions.x;
    }

    if (position.y < 0.0f) {
      position.y = 0.0f;
    }
    if (position.y + dimensions.y > screenHeight) {
      position.y = screenHeight - dimensions.y;
    }

    resetTimer += deltaTime;
    if (resetTimer > 0.05f) { 
      directionReversedThisFrame = false;
      resetTimer = 0.0f;
    }

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

    shootTimer += deltaTime;
    constexpr float shootInterval =
        0.5f; 
    if (shootTimer >= shootInterval && shootCooldown <= 0.0f && entitySpawner) {
      float randomValue =
          static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      constexpr float shootProbability = 0.05f; 

      if (randomValue < shootProbability) {
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
          const float bulletY = position.y + dimensions.y;

          Bullet *bullet = new Bullet(bulletX, bulletY, bulletWidth,
                                      bulletHeight, this, timeline, renderer);

          if (bullet != nullptr) {
            bullet->setComponent("isPlayerBullet", false);
            bullet->setComponent("screenHeight", screenHeight);

            bullet->EnablePhysics(false);

            constexpr float bulletSpeed = 1000.0f;
            bullet->SetVelocityY(bulletSpeed);
            bullet->SetVelocityX(0.0f);

            entitySpawner->AddEntity(bullet);

            setComponent("shootCooldown",
                         1.1f); 
          }
        }
      }

      shootTimer = 0.0f;
    }
  }

  void OnCollision(Entity *other, CollisionData *data) override {
    if (!other || !collisionEnabled) {
      return;
    }


    if (!other->collisionEnabled) {
      return; 
    }


    if (hasComponent("markedForDeletion") &&
        getComponent<bool>("markedForDeletion")) {
      return;
    }


    if (other->entityType == "Bullet") {
      bool isPlayerBullet = false;
      if (other->hasComponent("isPlayerBullet")) {
        isPlayerBullet = other->getComponent<bool>("isPlayerBullet");
      }
      if (isPlayerBullet) {
        setComponent("markedForDeletion", true);
      }
    }
  }


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
        false); 

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
    setComponent("shootCooldown", 0.0f); 
    setComponent("markedForDeath", false);
    setComponent("initialX", x); 
    setComponent("initialY", y);
    setComponent("initialWidth", w);
    setComponent("initialHeight", h);
  }

  void Update(float deltaTime, InputManager *input,
              EntityManager *entitySpawner) override {

    Bullet::ProcessPendingDeletions();

    Invader::ProcessPendingDeletions();

    bool markedForDeath = false;
    if (hasComponent("markedForDeath")) {
      markedForDeath = getComponent<bool>("markedForDeath");
    }

    if (markedForDeath) {
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

      float initialX = getComponent<float>("initialX");
      float initialY = getComponent<float>("initialY");
      float initialWidth = getComponent<float>("initialWidth");
      float initialHeight = getComponent<float>("initialHeight");

      position.x = initialX;
      position.y = initialY;
      dimensions.x = initialWidth;
      dimensions.y = initialHeight;

      SetVelocityX(0.0f);
      SetVelocityY(0.0f);

      setComponent("markedForDeath", false);

      rendering.isVisible = true;
      collisionEnabled = true;
    }

    float cooldown = getComponent<float>("shootCooldown");
    if (cooldown > 0.0f) {
      cooldown -= deltaTime;
      if (cooldown < 0.0f)
        cooldown = 0.0f;
      setComponent("shootCooldown", cooldown);
    }

    setComponent("dashLeftActive", false);
    setComponent("dashRightActive", false);

    float currentCooldown = getComponent<float>("shootCooldown");
    if (hasComponent("shouldShoot") && getComponent<bool>("shouldShoot") &&
        currentCooldown <= 0.0f) {
      SDL_Renderer *renderer = nullptr;

      if (hasComponent("rendererPtr")) {
        renderer = reinterpret_cast<SDL_Renderer *>(
            getComponent<void *>("rendererPtr"));
      }

      if (entitySpawner) {
        const float bulletWidth = 10.0f;
        const float bulletHeight = 10.0f;
        const float bulletX = position.x + (dimensions.x - bulletWidth) * 0.5f;
        const float bulletY = position.y - bulletHeight;

        Bullet *bullet = new Bullet(bulletX, bulletY, bulletWidth, bulletHeight,
                                    this, timeline, renderer);

        if (bullet != nullptr) {
          bullet->setComponent("isPlayerBullet", true);

          float screenHeight = 1000.0f;
          if (hasComponent("screenHeight")) {
            screenHeight = getComponent<float>("screenHeight");
          }
          bullet->setComponent("screenHeight", screenHeight);

          bullet->EnablePhysics(false);

          constexpr float bulletSpeed = -1000.0f; 
          bullet->SetVelocityY(bulletSpeed);
          bullet->SetVelocityX(0.0f);

          entitySpawner->AddEntity(bullet);

          constexpr float shootCooldownTime = 0.3f;
          setComponent("shootCooldown", shootCooldownTime);
        }

      }

      setComponent("shouldShoot", false);
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

    float screenWidth = getComponent<float>("screenWidth");
    if (position.x < 0.0f) {
      position.x = 0.0f;
    }
    if (position.x + dimensions.x > screenWidth) {
      position.x = screenWidth - dimensions.x;
    }
  }

  void ResetGame(EntityManager *entitySpawner);

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

inline void Player::ResetGame(EntityManager *entitySpawner) {
  SDL_Renderer *renderer = nullptr;
  if (hasComponent("rendererPtr")) {
    renderer = reinterpret_cast<SDL_Renderer *>(
        getComponent<void *>("rendererPtr"));
  }

  std::vector<Entity *> toRemove;
  for (Entity *e : entitySpawner->getEntityVectorRef()) {
    if (e && (e->entityType == "Invader_0" || e->entityType == "Invader_1" ||
              e->entityType == "Invader_2" || e->entityType == "Bullet")) {
      toRemove.push_back(e);
    }
  }
  for (Entity *entity : toRemove) {
    entitySpawner->RemoveEntity(entity);
  }

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

  const int rows = 3;
  const int cols = 8;
  const float invaderWidth = 32.0f;
  const float invaderHeight = 32.0f;
  const float gapX = 30.0f;
  const float gapY = 25.0f;
  float screenWidth = getComponent<float>("screenWidth");
  const float totalWidth = cols * invaderWidth + (cols - 1) * gapX;
  const float startX = (screenWidth - totalWidth) * 0.5f;
  const float startY = 100.0f;

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      float x = startX + c * (invaderWidth + gapX);
      float y = startY + r * (invaderHeight + gapY);
      int invaderType = (r + c) % 3;
      Invader *invader =
          new Invader(x, y, invaderWidth, invaderHeight, invaderType, timeline, renderer);
      entitySpawner->AddEntity(invader);
    }
  }
}
