# GeminiDX Engine

A custom 3D Graphics Engine built from scratch using C++23 and DirectX 11. This project is a learning exercise in building a modern rendering engine on Windows.

## Current Status

The engine is in the early stages of development. It can successfully initialize a Win32 window, set up the Direct3D 11 device and swap chain, and render a basic scene.

## Core Features

*   **Direct3D 11 Renderer:** Core rendering pipeline using D3D11.
*   **Win32 Windowing:** Custom window and message loop implementation.
*   **Component-Based Scene:** `GameObject` based scene graph.
*   **Shader System:** Runtime HLSL shader compilation (`.hlsl` files).
*   **Resource Management:** `ComPtr` is used for all DirectX COM interfaces to ensure safe resource handling.
*   **Basic Lighting:** Support for directional and point lights.
*   **Shadow Mapping:** Directional light shadow mapping.
*   **Skybox Rendering:** Basic skybox implementation.
*   **2D UI Rendering:** Simple sprite-based UI rendering.
*   **Model Loading:** Basic `.obj` model loading.