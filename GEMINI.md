Project Context: GeminiDX Engine

Project Overview

This project is a custom 3D Graphics Engine built from scratch using C++ and DirectX (Direct3D). The development environment is Visual Studio 2026 on Windows 11.

Tech Stack & Constraints

Language: C++23 (Standard ISO C++).

Graphics API: Direct3D 11 (Feature Level 11_0) initially, with architecture designed to support D3D12 later.

OS API: Win32 API (No external windowing libraries like GLFW/SDL).

Math Library: DirectXMath (<DirectXMath.h>).

Shader Language: HLSL (Shader Model 5.0).

IDE/Compiler: Visual Studio 2026 (MSVC).

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

Window Class: Encapsulates the HWND, message loop (WndProc), and window creation.

Graphics Class: Encapsulates the Device, Context, SwapChain, and RenderTargetView.

Game Class: The main entry point containing the specific logic, owning the Window and Graphics instances.

Development Phase: Initialization

We are currently in the setup phase. The immediate goals are:

Setting up a clean Win32 Window Class.

Initializing the Direct3D 11 Device and SwapChain.

Clearing the screen to a solid color (Deep Blue).

Common Snippet Patterns to Use

Win32 Main Entry:

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    // Logic here
}


ThrowIfFailed Helper:

inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        throw std::exception("DirectX Error");
    }
}


Vertex Struct:

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};


Excluded Libraries

Do not use: GLM, GLFW, GLUT, SDL2, or raw standard arrays (use std::vector).

Always update README.md and .gitignore if necessary.
