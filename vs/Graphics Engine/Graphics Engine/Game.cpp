#include "Game.h"
#include "Material.h"

Game::Game() {}

bool Game::Initialize(HINSTANCE hInstance, int nCmdShow)
{
    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 720;

    try
    {
        m_window.Initialize(hInstance, nCmdShow, L"GeminiDX Engine", L"GeminiDXWindowClass", WINDOW_WIDTH, WINDOW_HEIGHT);
        m_graphics.Initialize(m_window.GetHWND(), WINDOW_WIDTH, WINDOW_HEIGHT);
        m_input.Initialize(m_window.GetHWND());

        m_camera = std::make_unique<Camera>();
        m_camera->SetPosition(0.0f, 1.0f, -5.0f);

        // Create Materials
        auto shinyRedMat = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f), 2.0f, 64.0f);
        auto dullGrayMat = std::make_shared<Material>(DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), 0.1f, 8.0f);

        // Create GameObjects
        auto floor = std::make_unique<GameObject>(m_graphics.GetMeshAsset(), dullGrayMat);
        floor->SetPosition(0.0f, -2.0f, 0.0f);
        floor->SetScale(10.0f, 0.1f, 10.0f);
        m_gameObjects.push_back(std::move(floor));

        auto cube1 = std::make_unique<GameObject>(m_graphics.GetMeshAsset(), shinyRedMat);
        cube1->SetPosition(0.0f, 0.0f, 0.0f);
        m_gameObjects.push_back(std::move(cube1));
        
        auto cube2 = std::make_unique<GameObject>(m_graphics.GetMeshAsset(), shinyRedMat);
        cube2->SetPosition(2.0f, 0.0f, 2.0f);
        m_gameObjects.push_back(std::move(cube2));
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
        if (!m_window.ProcessMessages())
        {
            break;
        }

        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_lastTime).count();
        m_lastTime = currentTime;

        Update(deltaTime);
        Render();
    }
}

void Game::Update(float deltaTime)
{
    m_input.Update();

    if (m_input.IsKeyDown(VK_ESCAPE))
    {
        PostQuitMessage(0);
    }

    const float moveSpeed = 5.0f * deltaTime;
    const float rotSpeed = 0.5f * deltaTime;

    if (m_input.IsKeyDown('W')) m_camera->AdjustPosition(0.0f, 0.0f, moveSpeed);
    if (m_input.IsKeyDown('S')) m_camera->AdjustPosition(0.0f, 0.0f, -moveSpeed);
    if (m_input.IsKeyDown('A')) m_camera->AdjustPosition(-moveSpeed, 0.0f, 0.0f);
    if (m_input.IsKeyDown('D')) m_camera->AdjustPosition(moveSpeed, 0.0f, 0.0f);
    if (m_input.IsKeyDown(VK_SPACE)) m_camera->AdjustPosition(0.0f, moveSpeed, 0.0f);
    if (m_input.IsKeyDown(VK_SHIFT)) m_camera->AdjustPosition(0.0f, -moveSpeed, 0.0f);

    float mouseDx = static_cast<float>(m_input.GetMouseDeltaX()) * rotSpeed;
    float mouseDy = static_cast<float>(m_input.GetMouseDeltaY()) * rotSpeed;
    m_camera->AdjustRotation(mouseDy, mouseDx, 0.0f);

    // Animate one of the cubes
    static float rotation = 0.0f;
    rotation += 0.5f * deltaTime;
    m_gameObjects[2]->SetRotation(rotation, rotation, 0.0f);
}

void Game::Render()
{
    m_graphics.RenderFrame(m_camera.get(), m_gameObjects);
}
