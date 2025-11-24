#include "include/Game.h"
#include "include/AssetManager.h" 
#include "include/UIRenderer.h"
#include "include/Material.h"
#include "include/ModelLoader.h"
#include "include/TextureLoader.h"
#include "include/Graphics.h" 
#include "include/Collision.h"
#include "include/Player.h"
#include "include/PhysicsSystem.h"
#include "include/PostProcess.h"
#include "include/Renderer.h"

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
        LOG_INFO(isEnabled ? "Bloom: ON" : "Bloom: OFF");;
    }
    bKeyWasPressed = bKeyPressed;

    if (m_scene)
    {
        m_scene->Update(deltaTime, m_input);
    }
}

void Game::Render()
{
    if (m_scene)
    {
        m_scene->Render(m_renderer.get(), m_uiRenderer.get());
    }

    m_graphics.Present();
}