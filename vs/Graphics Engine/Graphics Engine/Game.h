#pragma once

#include "Window.h"
#include "Graphics.h"
#include "Input.h"
#include "Renderer.h"
#include "Camera.h"
#include "GameObject.h"
#include "Mesh.h" // Added to support vector<unique_ptr<Mesh>>
#include "SimpleFont.h" // Added SimpleFont
#include <memory>
#include <vector>
#include <chrono>

class Game
{
public:
    Game();
    ~Game() = default;

    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    void Run();

private:
    void Update(float deltaTime);
    void Render();

    Window m_window;
    Graphics m_graphics;
    std::unique_ptr<Renderer> m_renderer;
    Input m_input;
    SimpleFont m_font; // UI Font

    std::unique_ptr<Camera> m_camera;

    // Storage for the loaded raw mesh data
    std::vector<std::unique_ptr<Mesh>> m_meshAssets;

    // The actual objects in the scene
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;

    // Lighting
    DirectionalLight m_dirLight;
    std::vector<PointLight> m_pointLights;

    std::chrono::steady_clock::time_point m_lastTime;

    // FPS Counter
    int m_fps = 0;
    int m_frameCount = 0;
    float m_timeAccum = 0.0f;
};