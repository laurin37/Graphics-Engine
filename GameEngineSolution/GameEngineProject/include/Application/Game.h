#pragma once

#include <memory>
#include <vector>
#include <chrono>

#include "../Platform/Window.h"
#include "../Renderer/Graphics.h"
#include "../Input/Input.h"
#include "../Renderer/Renderer.h"
#include "../UI/UIRenderer.h"
#include "../Application/Scene.h"

class AssetManager; // Forward Declaration

class Game
{
public:
    Game();
    ~Game();

    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    void Run();

private:
    void Update(float deltaTime);
    void Render();

    // System Objects
    Window m_window;
    Graphics m_graphics;
    Input m_input;

    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<AssetManager> m_assetManager;
    std::unique_ptr<UIRenderer> m_uiRenderer;

    // Scene
    std::unique_ptr<Scene> m_scene;

    // Loop / Timing
    std::chrono::steady_clock::time_point m_lastTime;
};