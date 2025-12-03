# CSC581_GameEngine
<p align="center">
  <img src="https://img.shields.io/badge/CSC581-Game%20Engine%20Construction-%234285F4?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Mac-%236DB33F?style=for-the-badge&logo=windows&logoColor=white" />
  <img src="https://img.shields.io/badge/C++20-Enabled-%2300599C?style=for-the-badge&logo=cplusplus&logoColor=white" />
  <img src="https://img.shields.io/badge/SDL3-Powered-%23FF6F00?style=for-the-badge&logo=googlegames&logoColor=white" />
</p>

The **Game Engine Construction Project** is designed to provide comprehensive, hands-on experience in the **design, development, integration, and testing** of a functional game engine. In addition to team-based engine development, each student will implement an **individual game** using the shared engine to showcase system functionalities.  

---

## 🎮 Games Included

This engine includes three playable demo games:

| Game | Description |
|------|-------------|
| **🚀 Space Invaders** | Classic arcade shooter - destroy waves of alien invaders! |
| **🧱 Brick Breaker** | Break all the bricks using a bouncing ball and paddle! |
| **🦘 Platformer** | Jump between static and moving platforms in this challenging platformer! |

---

## 📦 Installation

### Prerequisites
- **CMake** 3.16 or higher
- **C++20** compatible compiler (VS 2022 on Windows, Clang/GCC on Mac/Linux)
- **Git** (for cloning with submodules)

### Clone & Build

```bash
# Clone the repository with all submodules
git clone https://github.com/TaylorBrown96/CSC581_GameEngine.git --recursive
cd CSC581_GameEngine

# Configure and build
cmake -S . -B build
cmake --build build --config Release
```

> [!NOTE]  
> **If submodules are missing** (e.g., you switched branches or downloaded a zip):
> ```bash
> git submodule update --init --recursive
> ```
> 
> **Or manually clone SDL:**
> ```bash
> git clone https://github.com/libsdl-org/SDL.git third_party/sdl
> ```

### Build Output

After building, the executables will be located in:
- **Windows:** `build/Release/`
- **Mac/Linux:** `build/`

The following executables are generated:

| Executable | Description |
|------------|-------------|
| `BrickServer` | Brick Breaker game server |
| `BrickClient` | Brick Breaker game client |
| `SpaceServer` | Space Invaders game server |
| `SpaceClient` | Space Invaders game client |
| `PlatformServer` | Platformer game server |
| `PlatformClient` | Platformer game client |

---

## 🚀 Launching the Games

Both games use a **client-server architecture**. You must start the server first, then connect one or more clients.

### 🧱 Brick Breaker

**Step 1: Start the Server**
```bash
# Windows
.\build\Release\BrickServer.exe

# Mac/Linux
./build/BrickServer
```

**Step 2: Start the Client** (in a new terminal)
```bash
# Windows
.\build\Release\BrickClient.exe

# Mac/Linux
./build/BrickClient
```

### 🚀 Space Invaders

**Step 1: Start the Server**
```bash
# Windows
.\build\Release\SpaceServer.exe

# Mac/Linux
./build/SpaceServer
```

**Step 2: Start the Client** (in a new terminal)
```bash
# Windows
.\build\Release\SpaceClient.exe

# Mac/Linux
./build/SpaceClient
```

### 🦘 Platformer

**Step 1: Start the Server**
```bash
# Windows
.\build\Release\PlatformServer.exe

# Mac/Linux
./build/PlatformServer
```

**Step 2: Start the Client** (in a new terminal)
```bash
# Windows
.\build\Release\PlatformClient.exe

# Mac/Linux
./build/PlatformClient
```

> [!TIP]
> **Multiplayer:** You can connect multiple clients to the same server! Each client gets their own player.

---

## 🎮 Controls

### Space Invaders

| Key | Action |
|-----|--------|
| `A` | Move Left |
| `D` | Move Right |
| `Space` | Shoot |
| `Shift + A` | Dash Left |
| `Shift + D` | Dash Right |
| `ESC` | Exit Game |

**Objective:** Destroy all the invaders before they reach you! You have 3 lives (hearts).

### Brick Breaker

| Key | Action |
|-----|--------|
| `A` | Move Paddle Left |
| `D` | Move Paddle Right |
| `Space` | Launch Ball |
| `Shift + A` | Dash Left |
| `Shift + D` | Dash Right |
| `ESC` | Exit Game |

**Objective:** Break all the bricks using the bouncing ball. Don't let the ball fall! You have 3 lives (hearts).

### Platformer

| Key | Action |
|-----|--------|
| `A` / `←` | Move Left |
| `D` / `→` | Move Right |
| `Space` | Jump |
| `P` | Pause/Unpause Timeline |
| `I` | Slow Down Timeline |
| `O` | Speed Up Timeline |
| `U` | Reset Timeline Speed |
| `ESC` | Exit Game |

**Objective:** Navigate between static and moving platforms. Don't fall off! If you fall below the screen, you'll respawn at the starting position. Jump on moving platforms to reach higher areas.

---

## 🔧 Network Configuration

Both games use **ZeroMQ** for networking on these default ports:

| Port | Purpose |
|------|---------|
| `5555` | Publisher (Server → Clients) |
| `5556` | Pull Socket (Clients → Server) |

The client connects to `localhost` by default. To connect to a remote server, modify the `serverAddress` in the respective `client_main.cpp` file.

---

## 🛠️ Key Engine Features

- **Entity-Component System** – Flexible game object architecture
- **Client-Server Networking** – ZeroMQ-powered multiplayer support
- **Physics & Collision System** – Built-in physics and collision detection
- **Event System** – Decoupled event-driven architecture
- **Replay System** – Record and playback game sessions
- **Job System** – Multithreaded task execution
- **Memory Pool** – Efficient memory management
- **Timeline System** – Game time management with pause/speed control

---

## 📁 Project Structure

```
GameEngine_M5/
├── src/                    # Core engine source code
│   ├── Core/               # GameEngine, Render, JobSystem
│   ├── Entities/           # Entity management
│   ├── Events/             # Event system
│   ├── Collision/          # Collision detection
│   ├── Physics/            # Physics simulation
│   ├── Input/              # Input handling
│   ├── Networking/         # Client/Server networking
│   ├── Replay/             # Replay recording/playback
│   ├── Memory/             # Memory pool
│   └── Timeline/           # Time management
├── demo_brickbreaker/      # Brick Breaker game
├── demo_space/             # Space Invaders game
├── game_cs/                # Platformer game
├── media/                  # Game assets (sprites, images)
├── third_party/            # External libraries (SDL3, ZeroMQ)
└── docs/                   # Project documentation
```

---

## Project Scenario & Process
Your team has been tasked with creating a **modular, reusable engine** that supports rapid development for multiple game genres.  
The engine will be built through **five major milestones**, each focusing on specific system features.  

### Milestones
1. **Build the Game Engine Foundations** (Modules 1–3)  
   - Core architecture, gameplay frameworks, and loop structures.  

2. **Integrate Conceptual Models** (Modules 4–6)  
   - Networking models, time management, and multithreading capabilities.  

3. **Implement Runtime Object Models** (Modules 7–9)  
   - Runtime object models using object-centric or property-centric approaches.  

4. **Construct Event Management Systems** (Modules 10–12)  
   - Event handling, synchronization, and system coordination.  

5. **Manage Game Engine Resources** (Modules 13–15)  
   - Resource management, input handling, and scripting support.  

---

## Deliverables & Assessment
Each milestone requires:  
- Team meetings for planning and task assignment.  
- Team-based contributions to engine design, implementation, and testing.  
- Individual development of a **mini-game** showcasing milestone features.  
- A **reflection paper** explaining contributions and decision-making.  

### Each Milestone Submission Includes
**Team Deliverables**:
- Updated engine source code with documentation and CMake build scripts.  
- A working build that compiles on **Ubuntu 24.04 LTS** or **Windows with VS 2022**.  

**Individual Deliverables**:
- A small playable mini-game demonstrating milestone features.  
- A **1–2 page reflection paper** on contributions, rationale, challenges, and future improvements.  
- **Appendix of screenshots** (not included in page count).  

### Grading Criteria (125 Points Total per Milestone)
- **50 pts** – Individual reflection writeup  
- **40 pts** – Functionality and correctness of engine features  
- **35 pts** – Team collaboration and documentation completeness  

> **Note:**   
> - Since we are CSC 581 students we will need to complete all components, including optional ones (graded out of 125 pts).  

---

## Team Ground Rules

### 1. Communication
- Primary platform: **Discord**  
- Major updates posted in **General** channel  
- Notify team if unavailable  

### 2. Meetings
- **Two weekly meetings** (virtual or in-person)  
- Agendas prepared in advance, notes recorded  
- Attendance expected; notify team if absent  

### 3. Work Distribution
- Tasks assigned equitably based on skill and workload  
- Members must meet deadlines  
- Ask for help early if struggling  

### 4. Code & Repository Standards
- Commit all code with **clear messages**  
- Ensure code compiles and runs on required platforms  
- Follow consistent naming, formatting, and documentation standards  

### 5. Accountability
- Contribute to both **engine development** and **individual game**  
- Write reflective papers individually (collaboration on ideas encouraged)  
- Missing deadlines or poor contributions will be addressed directly  

### 6. Collaboration & Respect
- Treat team members with professionalism and respect  
- Discuss differences constructively with problem-solving focus  
- Final decisions made by **consensus**; escalate unresolved issues to instructor  

### 7. Quality Assurance
- Test all code before merging to main branch  
- Review each milestone as a team before submission  
- Include code comments and milestone documentation as part of deliverables  

### 8. Academic Integrity
- Follow **University's Academic Integrity Policy**  
- No plagiarism or misuse of assets/libraries  
- Properly cite external resources  
