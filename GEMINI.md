Project Context: GeminiDX Engine

Project Overview

This project is a custom 3D Graphics Engine built from scratch using C++ and DirectX (Direct3D). The development environment is Visual Studio 2026 on Windows 11.

Tech Stack & Constraints

Language: C++23 (Standard ISO C++).

Graphics API: Direct3D 11 (Feature Level 11_0). Architecture is designed to support future migration to D3D12.

OS API: Win32 API (No external windowing libraries like GLFW/SDL).

Math Library: DirectXMath (<DirectXMath.h>).

Shader Language: HLSL (Shader Model 5.0).

IDE/Compiler: Visual Studio 2026 (MSVC).

Asset Loading:

Models: Custom OBJ Parser (supports positions, UVs, normals).

Textures: Native WIC (Windows Imaging Component).

Fonts: Custom 2D Sprite Batch (Bitmap Fonts).

Architectural Guidelines

1. Resource Management (CRITICAL)

NEVER use raw pointers for DirectX interfaces (e.g., ID3D11Device*).

ALWAYS use Microsoft::WRL::ComPtr<T> for all COM interfaces.

Example: Microsoft::WRL::ComPtr<ID3D11Device> m_device;

Do not manually call .Release(). Let the ComPtr handle scoping.

2. Error Handling

Use exceptions for fatal initialization errors.

Implement a helper function ThrowIfFailed(HRESULT hr) that throws a standard std::exception or custom runtime error if the HRESULT indicates failure.

3. Coding Style (Strict)

Classes: PascalCase (e.g., GraphicsEngine, MeshRenderer).

Variables: camelCase (e.g., vertexCount, shaderPath).

Member Variables: Prefix with m_ (e.g., m_swapChain, m_windowHandle).

Constants: UPPER_SNAKE_CASE (e.g., MAX_LIGHTS).

Strings: Use std::wstring and L"strings" for all Win32 API interactions (Window titles, file paths).

Headers: Use #pragma once.

4. Application Structure

Game Class: The main entry point. Owns the Window, Graphics, Input, Camera, and Scene Data (GameObjects). Contains the main loop (Run, Update, Render).

Graphics Class: The Renderer. Owns Device, Context, SwapChain. Handles the rendering pipeline (Shadow Pass -> Main Pass -> UI Pass). It should not own game logic.

GameObject Class: Represents an entity in the world. Has a Transform (Pos/Rot/Scale), a Mesh*, and a Material.

Input Class: Abstracts Win32 raw input into IsKeyDown and Mouse Delta queries.

Shader Classes: VertexShader and PixelShader wrappers that handle compilation and binding.

Current Features (Implemented)

Rendering:

Forward Rendering Pipeline.

Blinn-Phong Lighting Model (Diffuse + Specular + Ambient).

Normal Mapping (Tangent Space).

Dynamic Point Lights (with attenuation) + Directional Light.

Soft Shadows (PCF 3x3) via Shadow Mapping.

Skybox Rendering (Cube Map simulation).

Systems:

First-Person Camera (WASD + Mouse Look).

Asset Loading (OBJ Models, WIC Textures).

2D UI Overlay (Custom Sprite Font).

Material System (Support for Textures and Properties).

Roadmap: Future Goals

Phase 1: Visual Polish (Post-Processing)

Goal: Implement a Post-Processing pipeline.

Features:

Render Scene to Off-Screen Texture (RTT).

Full-screen Quad rendering.

Bloom (Gaussian Blur + Threshold).

Tone Mapping (ACES/Reinhard).

Gamma Correction.

Phase 2: Physics & Collision

Goal: Implement basic gameplay physics.

Features:

AABB (Axis-Aligned Bounding Box) generation for meshes.

Sphere-Box and Box-Box intersection tests.

Simple collision response (stop player from walking through walls).

Phase 3: Architecture Scalability

Goal: Move from hardcoded initialization to data-driven design.

Features:

ResourceManager: Prevent duplicate asset loading. Cache textures/meshes by filename.

Scene File: Load level layout (positions, lights) from a JSON or text file instead of C++ code.

Phase 4: Advanced Rendering

Goal: Push visual fidelity.

Features:

Deferred Rendering (Support 100+ lights).

PBR (Physically Based Rendering) Shaders.

Screen Space Ambient Occlusion (SSAO).

Common Snippet Patterns

Win32 Main Entry:

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    try {
        Game game;
        if (game.Initialize(hInstance, nCmdShow)) game.Run();
    } catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK);
    }
    return 0;
}


ThrowIfFailed Helper:

inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) { throw std::runtime_error("DirectX Error"); }
}


Vertex Struct:

struct Vertex {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uv;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT3 tangent;
};
