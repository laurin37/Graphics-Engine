#include "Window.h"
#include "Graphics.h"
#include <stdexcept>
#include <iostream>

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

        while (window.ProcessMessages())
        {
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
