#include "Application/Game.h"
#include "Events/ApplicationEvents.h"
#include "Events/InputEvents.h"
#include "Utils/Logger.h"

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
        m_window.SetEventBus(&m_eventBus);
        
        m_graphics.Initialize(m_window.GetHWND(), WINDOW_WIDTH, WINDOW_HEIGHT);
        m_input.Initialize(m_window.GetHWND());

        m_assetManager = std::make_unique<AssetManager>(&m_graphics);
        m_renderer = std::make_unique<Renderer>();
        m_renderer->Initialize(&m_graphics, m_assetManager.get(), WINDOW_WIDTH, WINDOW_HEIGHT);
        m_uiRenderer = std::make_unique<UIRenderer>(&m_graphics);

        m_scene = std::make_unique<Scene>(m_assetManager.get(), &m_graphics, &m_input, &m_eventBus);
        m_scene->Load();
        
        // Subscribe to events via EventBus
        SubscribeToEvents();
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

void Game::SubscribeToEvents()
{
    // Subscribe to Window Resize events
    auto resizeId = m_eventBus.Subscribe(EventType::WindowResize, [this](Event& e) {
        // WindowResizeEvent& event = static_cast<WindowResizeEvent&>(e);
        // m_graphics.Resize(event.GetWidth(), event.GetHeight()); // TODO: Implement Graphics::Resize
        // m_renderer->OnResize(event.GetWidth(), event.GetHeight()); // TODO: Implement Renderer::OnResize
    }, EventPriority::Normal);
    m_eventSubscriptions.push_back(resizeId);

    // Subscribe to Window Close events (High priority - app should handle first)
    auto closeId = m_eventBus.Subscribe(EventType::WindowClose, [this](Event& e) {
        PostQuitMessage(0);
        e.Handled = true;
    }, EventPriority::High);
    m_eventSubscriptions.push_back(closeId);

    // Subscribe to Key Pressed events (High priority - app-level shortcuts)
    auto keyId = m_eventBus.Subscribe(EventType::KeyPressed, [this](Event& e) {
        KeyPressedEvent& event = static_cast<KeyPressedEvent&>(e);
        
        // Skip key repeats (held keys generate repeatCount > 1)
        if (event.GetRepeatCount() > 1) return;

        switch (event.GetKeyCode())
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            e.Handled = true;
            break;
        case 'B':
            m_renderer->GetPostProcess()->ToggleBloom();
            LOG_INFO(m_renderer->GetPostProcess()->IsBloomEnabled() ? "Bloom: ON" : "Bloom: OFF");
            e.Handled = true;
            break;
        case 'H':
            m_showDebugCollision = !m_showDebugCollision;
            LOG_INFO(m_showDebugCollision ? "Debug Collision: ON" : "Debug Collision: OFF");
            e.Handled = true;
            break;
        case VK_F1:
            if (m_scene) {
                m_scene->ToggleDebugUI();
                LOG_INFO(m_scene->IsDebugUIEnabled() ? "Debug UI: ON" : "Debug UI: OFF");
            }
            e.Handled = true;
            break;
        }
    }, EventPriority::High);
    m_eventSubscriptions.push_back(keyId);
}