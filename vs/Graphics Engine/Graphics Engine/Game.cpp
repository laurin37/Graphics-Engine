#include "Game.h"
#include "Material.h"
#include "ModelLoader.h"
#include "TextureLoader.h"
#include "Graphics.h" 

Game::Game()
    : m_dirLight{ {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} }
{
}

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
        m_meshAssets.push_back(ModelLoader::Load(m_graphics.GetDevice(), "Assets/Models/basic/cube.obj"));
        m_meshAssets.push_back(ModelLoader::Load(m_graphics.GetDevice(), "Assets/Models/basic/cylinder.obj"));
        m_meshAssets.push_back(ModelLoader::Load(m_graphics.GetDevice(), "Assets/Models/basic/cone.obj"));
        m_meshAssets.push_back(ModelLoader::Load(m_graphics.GetDevice(), "Assets/Models/basic/sphere.obj"));
        m_meshAssets.push_back(ModelLoader::Load(m_graphics.GetDevice(), "Assets/Models/basic/torus.obj"));

        Mesh* meshCube = m_meshAssets[0].get();
        Mesh* meshCylinder = m_meshAssets[1].get();
        Mesh* meshCone = m_meshAssets[2].get();
        Mesh* meshSphere = m_meshAssets[3].get();
        Mesh* meshTorus = m_meshAssets[4].get();

        // 4. Load Textures
        auto texWood = TextureLoader::Load(m_graphics.GetDevice(), m_graphics.GetContext(), L"Assets/Textures/pine_bark_diff_4k.jpg");
        auto normWood = TextureLoader::Load(m_graphics.GetDevice(), m_graphics.GetContext(), L"Assets/Textures/pine_bark_disp_4k.png");
        auto texMetal = TextureLoader::Load(m_graphics.GetDevice(), m_graphics.GetContext(), L"Assets/Textures/blue_metal_plate_diff_4k.jpg");
        auto normMetal = TextureLoader::Load(m_graphics.GetDevice(), m_graphics.GetContext(), L"Assets/Textures/blue_metal_plate_disp_4k.png");

        // --- Font Loading Logic ---
        try {
            auto fontTex = TextureLoader::Load(m_graphics.GetDevice(), m_graphics.GetContext(), L"Assets/Textures/font.png");
            m_font.Initialize(fontTex);
        }
        catch (...) {
            // FALLBACK: If font.png isn't found, use wood texture so we see SOMETHING.
            // This confirms the pipeline works but the asset is missing.
            m_font.Initialize(texWood);
        }

        // 5. Create Materials
        auto matFloor = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.2f, 10.0f, texWood, normWood);
        auto matPillar = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.8f, 32.0f, texMetal, normMetal);
        auto matRoof = std::make_shared<Material>(DirectX::XMFLOAT4(0.8f, 0.1f, 0.1f, 1.0f), 0.8f, 32.0f);
        auto matGold = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.8f, 0.0f, 1.0f), 1.0f, 64.0f);
        auto matGlowing = std::make_shared<Material>(DirectX::XMFLOAT4(0.2f, 1.0f, 1.0f, 1.0f), 1.0f, 128.0f);

        // 6. Build Scene
        auto floor = std::make_unique<GameObject>(meshCube, matFloor);
        floor->SetPosition(0.0f, -1.0f, 0.0f);
        floor->SetScale(20.0f, 0.1f, 20.0f);
        m_gameObjects.push_back(std::move(floor));

        float pillarDist = 6.0f;
        float pillarPositions[4][2] = { {pillarDist, pillarDist}, {pillarDist, -pillarDist}, {-pillarDist, pillarDist}, {-pillarDist, -pillarDist} };

        for (int i = 0; i < 4; i++)
        {
            auto pillar = std::make_unique<GameObject>(meshCylinder, matPillar);
            pillar->SetPosition(pillarPositions[i][0], 1.0f, pillarPositions[i][1]);
            pillar->SetScale(1.0f, 2.0f, 1.0f);
            m_gameObjects.push_back(std::move(pillar));

            auto roof = std::make_unique<GameObject>(meshCone, matRoof);
            roof->SetPosition(pillarPositions[i][0], 3.5f, pillarPositions[i][1]);
            roof->SetScale(1.5f, 1.0f, 1.5f);
            m_gameObjects.push_back(std::move(roof));
        }

        auto pedestal = std::make_unique<GameObject>(meshCube, matPillar);
        pedestal->SetPosition(0.0f, 0.0f, 0.0f);
        pedestal->SetScale(2.0f, 1.0f, 2.0f);
        m_gameObjects.push_back(std::move(pedestal));

        auto artifact = std::make_unique<GameObject>(meshTorus, matGold);
        artifact->SetPosition(0.0f, 2.0f, 0.0f);
        artifact->SetScale(1.5f, 1.5f, 1.5f);
        artifact->SetRotation(DirectX::XM_PIDIV2, 0.0f, 0.0f);
        m_gameObjects.push_back(std::move(artifact));

        for (int i = 0; i < 4; i++)
        {
            auto orb = std::make_unique<GameObject>(meshSphere, matGlowing);
            orb->SetScale(0.5f, 0.5f, 0.5f);
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
    m_frameCount++;
    m_timeAccum += deltaTime;
    if (m_timeAccum >= 1.0f)
    {
        m_fps = m_frameCount;
        m_frameCount = 0;
        m_timeAccum -= 1.0f;
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
}

void Game::Render()
{
    m_graphics.RenderFrame(m_camera.get(), m_gameObjects, m_dirLight, m_pointLights);

    m_graphics.EnableUIState();

    float color[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
    m_font.DrawString(m_graphics, "FPS: " + std::to_string(m_fps), 10.0f, 10.0f, 30.0f, color);

    m_graphics.DisableUIState();
}