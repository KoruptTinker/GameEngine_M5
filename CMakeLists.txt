cmake_minimum_required(VERSION 3.16)
project(GameEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Put exes/dlls in build/<Config>
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

if(APPLE)
  list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
endif()

# ---------------- Packages ----------------
option(USE_VENDORED_SDL3 "Use vendored SDL3 library" ON)
option(USE_VENDORED_ZMQ  "Use vendored ZeroMQ library" ON)
option(USE_VENDORED_CPPZMQ "Use vendored cppzmq library" ON)

# SDL3
if(USE_VENDORED_SDL3)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdl/CMakeLists.txt")
    add_subdirectory(third_party/sdl EXCLUDE_FROM_ALL)
  else()
    find_package(SDL3 REQUIRED CONFIG)
  endif()
endif()

# ZeroMQ
if(USE_VENDORED_ZMQ)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libzmq/CMakeLists.txt")
    set(POLLER epoll)
    add_subdirectory(third_party/libzmq EXCLUDE_FROM_ALL)
  else()
    find_package(ZeroMQ REQUIRED CONFIG)
  endif()
endif()

# cppzmq
if(USE_VENDORED_CPPZMQ)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cppzmq/CMakeLists.txt")
    add_subdirectory(third_party/cppzmq EXCLUDE_FROM_ALL)
  else()
    find_package(cppzmq REQUIRED CONFIG)
  endif()
endif()

# ---------------- Engine sources ----------------
set(REQUIRED_SOURCES
  src/Entities/EntityManager.cpp
  src/main.cpp
  src/Core/GameEngine.cpp
  src/Networking/GameServer.cpp
  src/Networking/GameClient.cpp
  src/Input/Input.cpp
  src/Core/Render.cpp
  src/Physics/Physics.cpp
  src/Collision/Collisions.cpp
  src/Math/vec2.cpp
  src/Core/JobSystem.cpp
  src/Events/EventSystem.cpp
  src/Replay/ReplaySystem.cpp
  src/Memory/MemoryPool.cpp

)

set(ENGINE_SOURCES ${REQUIRED_SOURCES})

foreach(SOURCE_FILE ${OPTIONAL_SOURCES})
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}")
    list(APPEND ENGINE_SOURCES ${SOURCE_FILE})
  endif()
endforeach()

set(ENGINE_HEADERS
  src/Core/GameEngine.h
  src/Networking/GameServer.h
  src/Networking/GameClient.h
  src/Input/Input.h
  src/Core/Render.h
  src/Physics/Physics.h
  src/Collision/Collisions.h
  src/Entities/Entity.h
  src/Timeline/Timeline.h
  src/Core/SharedData.h
  src/Core/JobSystem.h
  src/Math/vec2.h
  src/Events/EventSystem.h
  src/Replay/ReplaySystem.h
  src/Memory/MemoryPool.h
  demo_brickbreaker/main.h
  demo_space/main.h
)

# ---------------- Core library (no main.cpp) ----------------
set(CORE_SOURCES ${ENGINE_SOURCES})
list(REMOVE_ITEM CORE_SOURCES src/main.cpp)

add_library(EngineCore STATIC
  ${CORE_SOURCES}
  ${ENGINE_HEADERS}
)
target_include_directories(EngineCore PUBLIC src demo_brickbreaker demo_space)
target_link_libraries(EngineCore PUBLIC SDL3::SDL3 libzmq cppzmq)

add_executable(BrickServer demo_brickbreaker/server_main.cpp)
target_include_directories(BrickServer PRIVATE src demo_brickbreaker)
target_link_libraries(BrickServer PRIVATE EngineCore)

add_executable(BrickClient demo_brickbreaker/client_main.cpp)
target_include_directories(BrickClient PRIVATE src demo_brickbreaker)
target_link_libraries(BrickClient PRIVATE EngineCore)

add_executable(SpaceServer demo_space/server_main.cpp)
target_include_directories(SpaceServer PRIVATE src demo_space)
target_link_libraries(SpaceServer PRIVATE EngineCore)

add_executable(SpaceClient demo_space/client_main.cpp)
target_include_directories(SpaceClient PRIVATE src demo_space)
target_link_libraries(SpaceClient PRIVATE EngineCore)

message(STATUS "=== GameEngine Build Configuration ===")
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "USE_VENDORED_SDL3: ${USE_VENDORED_SDL3}")
message(STATUS "=======================================")

