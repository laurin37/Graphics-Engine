#include "Application/Game.h"

#include "ResourceManagement/AssetManager.h" 
#include "UI/UIRenderer.h"
#include "Renderer/Material.h"
#include "ResourceManagement/ModelLoader.h"
#include "ResourceManagement/TextureLoader.h"
#include "Renderer/Graphics.h" 
#include "Physics/Collision.h"
#include "Renderer/PostProcess.h"
#include "Renderer/Renderer.h"

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
        m_window.Initialize(hInstance, nCmdShow, L"MyGameDemo", L"MyGameDemoClass", WINDOW_WIDTH, WINDOW_HEIGHT);
        m_graphics.Initialize(m_window.GetHWND(), WINDOW_WIDTH, WINDOW_HEIGHT);
        m_input.Initialize(m_window.GetHWND());

        m_assetManager = std::make_unique<AssetManager>(&m_graphics);
        m_renderer = std::make_unique<Renderer>();
        m_renderer->Initialize(&m_graphics, m_assetManager.get(), WINDOW_WIDTH, WINDOW_HEIGHT);
        m_uiRenderer = std::make_unique<UIRenderer>(&m_graphics);

        m_scene = std::make_unique<Scene>(m_assetManager.get(), &m_graphics, &m_input);
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

    // Quit Action
    if (m_input.IsActionDown(Action::Quit))
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

    // Toggle debug UI with F1 key
    static bool f1KeyWasPressed = false;
    bool f1KeyPressed = m_input.IsKeyDown(VK_F1);
    if (f1KeyPressed && !f1KeyWasPressed)
    {
        if (m_scene) {
            m_scene->ToggleDebugUI();
            LOG_INFO(m_scene->IsDebugUIEnabled() ? "Debug UI: ON" : "Debug UI: OFF");
        }
    }
    f1KeyWasPressed = f1KeyPressed;

    if (m_scene)
    {
        m_scene->Update(deltaTime);
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