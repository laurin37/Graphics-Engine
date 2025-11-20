#include "Window.h"
#include "Graphics.h"
#include <stdexcept>
#include <iostream>
#include <chrono>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    try
    {
        const int WINDOW_WIDTH = 1280;
        const int WINDOW_HEIGHT = 720;

        Window window;
        window.Initialize(hInstance, nCmdShow, L"GeminiDX Engine", L"GeminiDXWindowClass", WINDOW_WIDTH, WINDOW_HEIGHT);

        Graphics graphics;
        graphics.Initialize(window.GetHWND(), WINDOW_WIDTH, WINDOW_HEIGHT);
        
        Camera* camera = graphics.GetCamera();

        auto lastTime = std::chrono::high_resolution_clock::now();

        while (true)
        {
            if (!window.ProcessMessages())
            {
                break; // Exit loop if WM_QUIT is received
            }

            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
            lastTime = currentTime;

            const float moveSpeed = 5.0f * deltaTime;
            const float rotSpeed = 2.5f * deltaTime;

            if (GetAsyncKeyState('W') & 0x8000) camera->AdjustPosition(0.0f, 0.0f, moveSpeed);
            if (GetAsyncKeyState('S') & 0x8000) camera->AdjustPosition(0.0f, 0.0f, -moveSpeed);
            if (GetAsyncKeyState('A') & 0x8000) camera->AdjustPosition(-moveSpeed, 0.0f, 0.0f);
            if (GetAsyncKeyState('D') & 0x8000) camera->AdjustPosition(moveSpeed, 0.0f, 0.0f);
            if (GetAsyncKeyState(VK_SPACE) & 0x8000) camera->AdjustPosition(0.0f, moveSpeed, 0.0f);
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) camera->AdjustPosition(0.0f, -moveSpeed, 0.0f);

            if (GetAsyncKeyState(VK_UP) & 0x8000) camera->AdjustRotation(-rotSpeed, 0.0f, 0.0f);
            if (GetAsyncKeyState(VK_DOWN) & 0x8000) camera->AdjustRotation(rotSpeed, 0.0f, 0.0f);
            if (GetAsyncKeyState(VK_LEFT) & 0x8000) camera->AdjustRotation(0.0f, -rotSpeed, 0.0f);
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000) camera->AdjustRotation(0.0f, rotSpeed, 0.0f);

            graphics.RenderFrame();
        }
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    return 0;
}
