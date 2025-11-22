#include "Game.h"
#include "AssetManager.h"
#include "UIRenderer.h"
#include "Material.h"
#include "ModelLoader.h"
#include "TextureLoader.h"
#include "Graphics.h" 
#include "Collision.h"

Game::Game()
    : m_dirLight{ {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} }
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

        // Create the asset manager
        m_assetManager = std::make_unique<AssetManager>(&m_graphics);

        m_renderer = std::make_unique<Renderer>();
        m_renderer->Initialize(&m_graphics, m_assetManager.get(), WINDOW_WIDTH, WINDOW_HEIGHT);
        m_uiRenderer = std::make_unique<UIRenderer>(&m_graphics);

        // Load assets and set up the scene
        LoadScene();
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Initialization Failed", MB_OK | MB_ICONERROR);
        return false;
    }

    m_lastTime = std::chrono::steady_clock::now();
    return true;
}

void Game::LoadScene()
{
    // 1. Camera Setup
    m_camera = std::make_unique<Camera>();
    m_camera->SetPosition(0.0f, 5.0f, -15.0f);
    m_camera->AdjustRotation(0.3f, 0.0f, 0.0f);

    // 2. Setup Lights
    m_dirLight.direction = { 0.5f, -0.7f, 0.5f, 0.0f };
    m_dirLight.color = { 0.2f, 0.2f, 0.3f, 1.0f };

    m_pointLights.resize(MAX_POINT_LIGHTS);
    m_pointLights[0].position = { 0.0f, 0.0f, 0.0f, 15.0f };
    m_pointLights[0].color = { 1.0f, 0.0f, 0.0f, 2.0f };
    m_pointLights[0].attenuation = { 0.2f, 0.2f, 0.0f, 0.0f };

    m_pointLights[1].position = { 0.0f, 0.0f, 0.0f, 15.0f };
    m_pointLights[1].color = { 0.0f, 1.0f, 0.0f, 2.0f };
    m_pointLights[1].attenuation = { 0.2f, 0.2f, 0.0f, 0.0f };

    m_pointLights[2].position = { 0.0f, 0.0f, 0.0f, 15.0f };
    m_pointLights[2].color = { 0.0f, 0.0f, 1.0f, 2.0f };
    m_pointLights[2].attenuation = { 0.2f, 0.2f, 0.0f, 0.0f };

    m_pointLights[3].position = { 0.0f, 0.0f, 0.0f, 15.0f };
    m_pointLights[3].color = { 1.0f, 0.5f, 0.0f, 2.0f };
    m_pointLights[3].attenuation = { 0.2f, 0.2f, 0.0f, 0.0f };

    // 3. Load Basic Assets
    auto meshCube = m_assetManager->LoadMesh("Assets/Models/basic/cube.obj");
    auto meshCylinder = m_assetManager->LoadMesh("Assets/Models/basic/cylinder.obj");
    auto meshCone = m_assetManager->LoadMesh("Assets/Models/basic/cone.obj");
    auto meshSphere = m_assetManager->LoadMesh("Assets/Models/basic/sphere.obj");
    auto meshTorus = m_assetManager->LoadMesh("Assets/Models/basic/torus.obj");

    // 4. Load Textures
    auto texWood = m_assetManager->LoadTexture(L"Assets/Textures/pine_bark_diff_4k.jpg");
    auto normWood = m_assetManager->LoadTexture(L"Assets/Textures/pine_bark_disp_4k.png");
    auto texMetal = m_assetManager->LoadTexture(L"Assets/Textures/blue_metal_plate_diff_4k.jpg");
    auto normMetal = m_assetManager->LoadTexture(L"Assets/Textures/blue_metal_plate_disp_4k.png");

    // --- GENERATE DEBUG FONT ---
    auto debugFontTex = TextureLoader::CreateDebugFont(m_graphics.GetDevice().Get(), m_graphics.GetContext().Get());
    m_font.Initialize(debugFontTex);

    // 5. Create Materials
    auto matFloor = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.2f, 10.0f, texWood, normWood);
    auto matPillar = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.8f, 32.0f, texMetal, normMetal);
    auto matRoof = std::make_shared<Material>(DirectX::XMFLOAT4(0.8f, 0.1f, 0.1f, 1.0f), 0.8f, 32.0f);
    auto matGold = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.8f, 0.0f, 1.0f), 1.0f, 64.0f);
    auto matGlowing = std::make_shared<Material>(DirectX::XMFLOAT4(0.2f, 1.0f, 1.0f, 1.0f), 1.0f, 128.0f);

    // 6. Build Scene
    auto floor = std::make_unique<GameObject>(meshCube.get(), matFloor);
    floor->SetPosition(0.0f, -1.0f, 0.0f);
    floor->SetScale(20.0f, 0.1f, 20.0f);
    m_gameObjects.push_back(std::move(floor));

    float pillarDist = 6.0f;
    float pillarPositions[4][2] = { {pillarDist, pillarDist}, {pillarDist, -pillarDist}, {-pillarDist, pillarDist}, {-pillarDist, -pillarDist} };

    for (int i = 0; i < 4; i++)
    {
        auto pillar = std::make_unique<GameObject>(meshCylinder.get(), matPillar);
        pillar->SetPosition(pillarPositions[i][0], 1.0f, pillarPositions[i][1]);
        pillar->SetScale(1.0f, 2.0f, 1.0f);
        m_gameObjects.push_back(std::move(pillar));

        auto roof = std::make_unique<GameObject>(meshCone.get(), matRoof);
        roof->SetPosition(pillarPositions[i][0], 3.5f, pillarPositions[i][1]);
        roof->SetScale(1.5f, 1.0f, 1.5f);
        m_gameObjects.push_back(std::move(roof));
    }

    auto pedestal = std::make_unique<GameObject>(meshCube.get(), matPillar);
    pedestal->SetPosition(0.0f, 0.0f, 0.0f);
    pedestal->SetScale(2.0f, 1.0f, 2.0f);
    m_gameObjects.push_back(std::move(pedestal));

    auto artifact = std::make_unique<GameObject>(meshTorus.get(), matGold);
    artifact->SetPosition(0.0f, 2.0f, 0.0f);
    artifact->SetScale(1.5f, 1.5f, 1.5f);
    artifact->SetRotation(DirectX::XM_PIDIV2, 0.0f, 0.0f);
    m_gameObjects.push_back(std::move(artifact));

    for (int i = 0; i < 4; i++)
    {
        auto orb = std::make_unique<GameObject>(meshSphere.get(), matGlowing);
        orb->SetScale(0.5f, 0.5f, 0.5f);
        m_gameObjects.push_back(std::move(orb));
    }
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
            break; // Exit game loop on error
        }
    }
}

void Game::Update(float deltaTime)
{
    m_frameCount++;
    m_timeAccum += deltaTime;
    if (m_timeAccum >= 1.0f)
    {
        m_fps = m_frameCount;
        m_frameCount = 0;
        m_timeAccum -= 1.0f;
    }

    // --- Store last "safe" positions before update ---
    m_lastPositions.resize(m_gameObjects.size());
    for (size_t i = 0; i < m_gameObjects.size(); ++i)
    {
        m_lastPositions[i] = m_gameObjects[i]->GetPosition();
    }

    m_input.Update();

    if (m_input.IsKeyDown(VK_ESCAPE)) PostQuitMessage(0);

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

    static float time = 0.0f;
    time += deltaTime;

    if (m_gameObjects.size() > 10)
    {
        m_gameObjects[10]->SetRotation(DirectX::XM_PIDIV2, time, 0.0f);
    }

    if (m_gameObjects.size() >= 14)
    {
        for (int i = 0; i < 4; i++)
        {
            float offset = i * (DirectX::XM_PI / 2.0f);
            float radius = 3.5f;
            float x = sin(time + offset) * radius;
            float z = cos(time + offset) * radius;
            float y = 2.0f + sin(time * 2.0f + offset) * 0.5f;

            m_gameObjects[11 + i]->SetPosition(x, y, z);

            m_pointLights[i].position.x = x;
            m_pointLights[i].position.y = y;
            m_pointLights[i].position.z = z;
        }
    }

    UpdatePhysics(deltaTime);
}

void Game::UpdatePhysics(float deltaTime)
{
    // Simple N^2 collision detection
    for (size_t i = 0; i < m_gameObjects.size(); ++i)
    {
        for (size_t j = i + 1; j < m_gameObjects.size(); ++j)
        {
            auto& objA = m_gameObjects[i];
            auto& objB = m_gameObjects[j];

            // Skip check if one of the objects is the floor
            if (i == 0 || j == 0) continue;

            AABB boxA = objA->GetWorldBoundingBox();
            AABB boxB = objB->GetWorldBoundingBox();

            if (AABBIntersects(boxA, boxB))
            {
                // Collision detected, revert BOTH to last safe position
                const auto& posA = m_lastPositions[i];
                objA->SetPosition(posA.x, posA.y, posA.z);

                const auto& posB = m_lastPositions[j];
                objB->SetPosition(posB.x, posB.y, posB.z);
            }
        }
    }
}

void Game::Render()
{
    m_renderer->RenderFrame(*m_camera, m_gameObjects, m_dirLight, m_pointLights);
    m_renderer->RenderDebug(*m_camera, m_gameObjects);

    m_uiRenderer->EnableUIState();

    float color[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
    m_uiRenderer->DrawString(m_font, "FPS: " + std::to_string(m_fps), 10.0f, 10.0f, 30.0f, color);

    m_uiRenderer->DisableUIState();

    m_graphics.Present();
}