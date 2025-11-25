#include "../../include/Application/Game.h"
#include "../../include/ResourceManagement/AssetManager.h" 
#include "../../include/UI/UIRenderer.h"
#include "../../include/Renderer/Material.h"
#include "../../include/ResourceManagement/ModelLoader.h"
#include "../../include/ResourceManagement/TextureLoader.h"
#include "../../include/Renderer/Graphics.h" 
#include "../../include/Physics/Collision.h"
#include "../../include/EntityComponentSystem/Player.h"
#include "../../include/Physics/PhysicsSystem.h"
#include "../../include/Renderer/PostProcess.h"
#include "../../include/Renderer/Renderer.h"

Game::Game()
{
}

Game::~Game() = default;

bool Game::Initialize(HINSTANCE hInstance, int nCmdShow)
{
    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 720;

    ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

    try
    {
        m_window.Initialize(hInstance, nCmdShow, L"GeminiDX Engine", L"GeminiDXWindowClass", WINDOW_WIDTH, WINDOW_HEIGHT);
        m_graphics.Initialize(m_window.GetHWND(), WINDOW_WIDTH, WINDOW_HEIGHT);
        m_input.Initialize(m_window.GetHWND());

        m_assetManager = std::make_unique<AssetManager>(&m_graphics);
        m_renderer = std::make_unique<Renderer>();
        m_renderer->Initialize(&m_graphics, m_assetManager.get(), WINDOW_WIDTH, WINDOW_HEIGHT);
        m_uiRenderer = std::make_unique<UIRenderer>(&m_graphics);

        m_scene = std::make_unique<Scene>(m_assetManager.get(), &m_graphics);
        m_scene->Load();
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Initialization Failed", MB_OK | MB_ICONERROR);
        return false;
    }

    m_lastTime = std::chrono::steady_clock::now();
    return true;
}

void Game::Run()
{
    while (true)
    {
        if (!m_window.ProcessMessages()) break;

        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_lastTime).count();
        m_lastTime = currentTime;

        try
        {
            Update(deltaTime);
            Render();
        }
        catch (const std::exception& e)
        {
            MessageBoxA(nullptr, e.what(), "Runtime Error", MB_OK | MB_ICONERROR);
            break;
        }
    }
}

void Game::Update(float deltaTime)
{
    m_input.Update();

    // ESC to quit
    if (m_input.IsKeyDown(VK_ESCAPE))
    {
        PostQuitMessage(0);
    }

    // Toggle bloom with B key
    static bool bKeyWasPressed = false;
    bool bKeyPressed = m_input.IsKeyDown('B');
    if (bKeyPressed && !bKeyWasPressed)
    {
        m_renderer->GetPostProcess()->ToggleBloom();
        bool isEnabled = m_renderer->GetPostProcess()->IsBloomEnabled();
        LOG_INFO(isEnabled ? "Bloom: ON" : "Bloom: OFF");
    }
    bKeyWasPressed = bKeyPressed;

    // Toggle debug collision with H key
    static bool hKeyWasPressed = false;
    bool hKeyPressed = m_input.IsKeyDown('H');
    if (hKeyPressed && !hKeyWasPressed)
    {
        m_showDebugCollision = !m_showDebugCollision;
        LOG_INFO(m_showDebugCollision ? "Debug Collision: ON" : "Debug Collision: OFF");
    }
    hKeyWasPressed = hKeyPressed;

    if (m_scene)
    {
        m_scene->Update(deltaTime, m_input);
    }
}

void Game::Render()
{
    if (m_scene)
    {
        m_scene->Render(m_renderer.get(), m_uiRenderer.get(), m_showDebugCollision);
    }

    m_graphics.Present();
}