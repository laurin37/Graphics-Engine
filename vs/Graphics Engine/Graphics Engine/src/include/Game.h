#pragma once

#include "Window.h"
#include "Graphics.h"
#include "Input.h"
#include "Renderer.h"
#include "Camera.h"
#include "GameObject.h"
#include "Mesh.h" // Added to support vector<unique_ptr<Mesh>>
#include "SimpleFont.h"
#include "UIRenderer.h"
#include <memory>
#include <vector>
#include <chrono>

class AssetManager; // Forward Declaration
class UIRenderer;   // Forward Declaration

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
    void LoadScene();
    void UpdatePhysics(float deltaTime);

    Window m_window;
    Graphics m_graphics;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<AssetManager> m_assetManager;
    std::unique_ptr<UIRenderer> m_uiRenderer;
    Input m_input;
    SimpleFont m_font; // UI Font

    std::unique_ptr<Camera> m_camera;

    // The actual objects in the scene
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;

    // Lighting
    DirectionalLight m_dirLight;
    std::vector<PointLight> m_pointLights;

    std::vector<DirectX::XMFLOAT3> m_lastPositions;

    std::chrono::steady_clock::time_point m_lastTime;

    // FPS Counter
    int m_fps = 0;
    int m_frameCount = 0;
    float m_timeAccum = 0.0f;
};