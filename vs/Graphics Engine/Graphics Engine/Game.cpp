#include "Game.h"
#include "Material.h"
#include "ModelLoader.h"
#include "TextureLoader.h"
#include "Graphics.h" // For ThrowIfFailed

Game::Game() {}

bool Game::Initialize(HINSTANCE hInstance, int nCmdShow)
{
    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 720;

    // Initialize COM for WIC
    ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

    try
    {
        m_window.Initialize(hInstance, nCmdShow, L"GeminiDX Engine", L"GeminiDXWindowClass", WINDOW_WIDTH, WINDOW_HEIGHT);
        m_graphics.Initialize(m_window.GetHWND(), WINDOW_WIDTH, WINDOW_HEIGHT);
        m_input.Initialize(m_window.GetHWND());

        // 1. Camera Setup
        m_camera = std::make_unique<Camera>();
        m_camera->SetPosition(0.0f, 5.0f, -15.0f); // Look slightly down at the scene
        m_camera->AdjustRotation(0.3f, 0.0f, 0.0f); // Tilt down

        // 2. Load Basic Assets
        // Note: We store them in m_meshAssets so they stay alive for the GameObjects to reference
        m_meshAssets.push_back(ModelLoader::Load(m_graphics.GetDevice(), "Assets/Models/basic/cube.obj"));      // Index 0
        m_meshAssets.push_back(ModelLoader::Load(m_graphics.GetDevice(), "Assets/Models/basic/cylinder.obj"));  // Index 1
        m_meshAssets.push_back(ModelLoader::Load(m_graphics.GetDevice(), "Assets/Models/basic/cone.obj"));      // Index 2
        m_meshAssets.push_back(ModelLoader::Load(m_graphics.GetDevice(), "Assets/Models/basic/sphere.obj"));    // Index 3
        m_meshAssets.push_back(ModelLoader::Load(m_graphics.GetDevice(), "Assets/Models/basic/torus.obj"));     // Index 4

        Mesh* meshCube = m_meshAssets[0].get();
        Mesh* meshCylinder = m_meshAssets[1].get();
        Mesh* meshCone = m_meshAssets[2].get();
        Mesh* meshSphere = m_meshAssets[3].get();
        Mesh* meshTorus = m_meshAssets[4].get();

        // 3. Load Textures
        auto texWood = TextureLoader::Load(m_graphics.GetDevice(), m_graphics.GetContext(), L"Assets/Textures/pine_bark_diff_4k.jpg");
        auto texMetal = TextureLoader::Load(m_graphics.GetDevice(), m_graphics.GetContext(), L"Assets/Textures/blue_metal_plate_diff_4k.jpg");

        // 4. Create Materials
        auto matFloor = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.2f, 10.0f, texWood);   // Textured Wood
        auto matPillar = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.8f, 32.0f, texMetal); // Textured Metal
        auto matRoof = std::make_shared<Material>(DirectX::XMFLOAT4(0.8f, 0.1f, 0.1f, 1.0f), 0.8f, 32.0f);  // Shiny Red (No Texture)
        auto matGold = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.8f, 0.0f, 1.0f), 1.0f, 64.0f);  // Gold (No Texture)
        auto matGlowing = std::make_shared<Material>(DirectX::XMFLOAT4(0.2f, 1.0f, 1.0f, 1.0f), 1.0f, 128.0f); // Cyan (No Texture)

        // 5. Build Scene

        // -- The Floor --
        auto floor = std::make_unique<GameObject>(meshCube, matFloor);
        floor->SetPosition(0.0f, -1.0f, 0.0f);
        floor->SetScale(20.0f, 0.1f, 20.0f);
        m_gameObjects.push_back(std::move(floor));

        // -- The 4 Pillars (Cylinder + Cone) --
        float pillarDist = 6.0f;
        float pillarPositions[4][2] = { {pillarDist, pillarDist}, {pillarDist, -pillarDist}, {-pillarDist, pillarDist}, {-pillarDist, -pillarDist} };

        for (int i = 0; i < 4; i++)
        {
            // Base (Cylinder)
            auto pillar = std::make_unique<GameObject>(meshCylinder, matPillar);
            pillar->SetPosition(pillarPositions[i][0], 1.0f, pillarPositions[i][1]);
            pillar->SetScale(1.0f, 2.0f, 1.0f);
            m_gameObjects.push_back(std::move(pillar));

            // Roof (Cone)
            auto roof = std::make_unique<GameObject>(meshCone, matRoof);
            roof->SetPosition(pillarPositions[i][0], 3.5f, pillarPositions[i][1]); // Sit on top of cylinder
            roof->SetScale(1.5f, 1.0f, 1.5f);
            m_gameObjects.push_back(std::move(roof));
        }

        // -- Central Pedestal (Cube) --
        auto pedestal = std::make_unique<GameObject>(meshCube, matPillar); // Use metal material
        pedestal->SetPosition(0.0f, 0.0f, 0.0f);
        pedestal->SetScale(2.0f, 1.0f, 2.0f);
        m_gameObjects.push_back(std::move(pedestal));

        // -- The Artifact (Torus) --
        // Note: This is the object we will rotate in Update()
        auto artifact = std::make_unique<GameObject>(meshTorus, matGold);
        artifact->SetPosition(0.0f, 2.0f, 0.0f);
        artifact->SetScale(1.5f, 1.5f, 1.5f);
        artifact->SetRotation(DirectX::XM_PIDIV2, 0.0f, 0.0f); // Stand up
        m_gameObjects.push_back(std::move(artifact));

        // -- Orbiting Orbs (Spheres) --
        for (int i = 0; i < 4; i++)
        {
            auto orb = std::make_unique<GameObject>(meshSphere, matGlowing);
            orb->SetScale(0.5f, 0.5f, 0.5f);
            // Positions will be updated in the loop
            m_gameObjects.push_back(std::move(orb));
        }
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

        Update(deltaTime);
        Render();
    }
}

void Game::Update(float deltaTime)
{
    m_input.Update();

    if (m_input.IsKeyDown(VK_ESCAPE)) PostQuitMessage(0);

    // Camera Movement
    const float moveSpeed = 10.0f * deltaTime;
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

    // --- Scene Animation ---
    static float time = 0.0f;
    time += deltaTime;

    // 1. Rotate the Central Torus (The Artifact) - It is at index 10 (1 floor + 8 pillars parts + 1 pedestal)
    if (m_gameObjects.size() > 10)
    {
        m_gameObjects[10]->SetRotation(DirectX::XM_PIDIV2, time, 0.0f); // Spin on Y
    }

    // 2. Orbit the Spheres - They are the last 4 objects
    if (m_gameObjects.size() >= 14)
    {
        for (int i = 0; i < 4; i++)
        {
            float offset = i * (DirectX::XM_PI / 2.0f); // 90 degree offset per sphere
            float radius = 3.5f;
            float x = sin(time + offset) * radius;
            float z = cos(time + offset) * radius;
            float y = 2.0f + sin(time * 2.0f + offset) * 0.5f; // Bob up and down

            // Indices 11, 12, 13, 14
            m_gameObjects[11 + i]->SetPosition(x, y, z);
        }
    }
}

void Game::Render()
{
    m_graphics.RenderFrame(m_camera.get(), m_gameObjects);
}