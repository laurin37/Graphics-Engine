![Build Status](https://github.com/laurin37/Graphics-Engine/actions/workflows/build.yml/badge.svg)
# Game Engine

A custom 3D Graphics Engine built from scratch using **C++23** and **DirectX 11**. This project demonstrates a modern, data-driven architecture using an Entity-Component-System (ECS).

## Key Features

### Architecture
*   **Entity-Component-System (ECS)**: Custom ECS implementation for high-performance and modular game logic.
*   **Data-Driven Design**: Scenes, entities, and configurations are loaded from JSON files (`default.json`).
*   **Input Abstraction**: Action Mapping system decoupling logic from specific key codes.

### Rendering
*   **DirectX 11**: Native D3D11 implementation.
*   **Lighting**: Blinn-Phong shading, Directional Lights, Point Lights with attenuation.
*   **Shadows**: Directional Light Shadow Mapping with PCF (Percentage Closer Filtering).
*   **Materials**: Support for Diffuse, Specular, and Normal maps.
*   **Post-Processing**: Bloom effect (Gaussian Blur + Threshold).
*   **Skybox**: Cube map rendering.
*   **UI**: Custom 2D Debug UI and Sprite rendering.

### Physics & Gameplay
*   **Physics**: AABB (Axis-Aligned Bounding Box) collision detection and resolution.
*   **Weapon System**: Hitscan (Primary) and Projectile (Secondary) weapons with separate ammo pools.
*   **Mechanics**: Jumping, Gravity, Reloading, Health/Damage system.

## Controls

| Action | Key / Input |
| :--- | :--- |
| **Move** | `W`, `A`, `S`, `D` |
| **Look** | Mouse |
| **Jump** | `Space` |
| **Fire** | `Left Mouse Button` |
| **Alt Fire** | `Right Mouse Button` (Grenade) |
| **Reload** | `R` |
| **Toggle Bloom** | `B` |
| **Debug Collision** | `H` |
| **Debug UI** | `F1` |
| **Quit** | `Esc` |

## Tech Stack

*   **Language**: C++23
*   **Graphics API**: DirectX 11
*   **OS API**: Win32 API (No external windowing libraries like GLFW/SDL)
*   **Math**: DirectXMath
*   **Build System**: CMake

## Building the Project

### Prerequisites
*   Visual Studio 2022 (or newer) with C++ Desktop Development workload.
*   CMake 3.20+.

### Instructions
1.  Clone the repository.
2.  Open the folder in Visual Studio (or generate solution via CMake).
3.  Build the `GeminiDXEngine` target.
4.  Run the executable.

## Roadmap

### Core Architecture
*   [x] ECS Architecture
*   [x] JSON Scene Loading
*   [x] Input Abstraction
*   [ ] **Event Bus System** (Decoupled communication)
*   [ ] Memory Management (Custom Allocators)
*   [ ] Multithreading (Job System)

### Rendering
*   [ ] **Deferred Rendering Pipeline** (G-Buffer, Light Pass)
*   [ ] Physically Based Rendering (PBR)
*   [ ] Advanced Post-Processing (Tone Mapping, Gamma Correction, SSAO)
*   [ ] Particle System (CPU/GPU based)
*   [ ] Terrain Rendering (Heightmaps, LOD)
*   [ ] Skeletal Animation Support

### Physics & Gameplay
*   [ ] **Spatial Partitioning** (Octree/BVH for optimization)
*   [ ] Rigid Body Dynamics (Mass, Friction, Restitution)
*   [ ] Collision Resolution (Impulse-based)
*   [ ] Raycasting Improvements
*   [ ] Navigation Mesh / Pathfinding (A*)

### Audio
*   [ ] 3D Spatial Audio System
*   [ ] Audio Mixer & Channels

### Tools
*   [ ] In-Game Editor
*   [ ] Asset Hot-Reloading
*   [ ] Performance Profiler
