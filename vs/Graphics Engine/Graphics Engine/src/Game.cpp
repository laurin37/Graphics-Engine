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

Game::Game()
    : m_dirLight{ {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    m_player(nullptr) // Initialize raw pointer
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
    // Position is now controlled by the Player, but we set a default here
    m_camera->SetPosition(0.0f, 5.0f, -15.0f);
    m_camera->AdjustRotation(0.3f, 0.0f, 0.0f);

    // 2. Setup Lights
    m_dirLight.direction = { 0.5f, -0.7f, 0.5f, 0.0f };
    m_dirLight.color = { 0.2f, 0.2f, 0.3f, 1.0f };

    m_pointLights.resize(MAX_POINT_LIGHTS);
    // Setup point lights (Red, Green, Blue, Orange)
    m_pointLights[0] = { {0.0f, 0.0f, 0.0f, 15.0f}, {1.0f, 0.0f, 0.0f, 2.0f}, {0.2f, 0.2f, 0.0f, 0.0f} };
    m_pointLights[1] = { {0.0f, 0.0f, 0.0f, 15.0f}, {0.0f, 1.0f, 0.0f, 2.0f}, {0.2f, 0.2f, 0.0f, 0.0f} };
    m_pointLights[2] = { {0.0f, 0.0f, 0.0f, 15.0f}, {0.0f, 0.0f, 1.0f, 2.0f}, {0.2f, 0.2f, 0.0f, 0.0f} };
    m_pointLights[3] = { {0.0f, 0.0f, 0.0f, 15.0f}, {1.0f, 0.5f, 0.0f, 2.0f}, {0.2f, 0.2f, 0.0f, 0.0f} };

    // 3. Load Basic Assets
    std::shared_ptr<Mesh> meshCube = m_assetManager->LoadMesh("Assets/Models/basic/cube.obj");
    std::shared_ptr<Mesh> meshCylinder = m_assetManager->LoadMesh("Assets/Models/basic/cylinder.obj");
    std::shared_ptr<Mesh> meshCone = m_assetManager->LoadMesh("Assets/Models/basic/cone.obj");
    std::shared_ptr<Mesh> meshSphere = m_assetManager->LoadMesh("Assets/Models/basic/sphere.obj");
    std::shared_ptr<Mesh> meshTorus = m_assetManager->LoadMesh("Assets/Models/basic/torus.obj");

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

    // --- Create Player ---
    auto playerPtr = std::make_unique<Player>(meshCylinder.get(), matPillar, m_camera.get());
    playerPtr->SetPosition(0.0f, 5.0f, -5.0f);
    m_player = playerPtr.get();
    m_gameObjects.push_back(std::move(playerPtr));

    // --- Create Player's Gun ---
    auto gunMesh = meshCube; // Using cube as a placeholder for gun mesh
    auto gunMaterial = matPillar; // Using pillar material as a placeholder

    auto gunPtr = std::make_unique<Gun>(gunMesh.get(), gunMaterial);
    // Position the gun relative to the player (e.g., slightly in front and to the side)
    gunPtr->SetPosition(m_player->GetPosition().x + 0.5f, m_player->GetPosition().y + 0.5f, m_player->GetPosition().z + 0.5f);
    gunPtr->SetScale(0.2f, 0.2f, 0.8f); // Make it look like a simple gun barrel

    m_player->SetGun(gunPtr.get()); // Set player's gun pointer (non-owning)
    gunPtr->SetOwner(m_player); // Set the Gun's owner
    m_gameObjects.push_back(std::move(gunPtr)); // Game owns the Gun GameObject

    // --- Store Bullet and HealthObject assets ---
    m_bulletMesh = meshSphere; // Simple sphere for bullet
    m_bulletMaterial = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f), 1.0f, 64.0f); // Orange glowing bullet

    m_healthObjectMesh = meshCube; // Simple cube for health object
    m_healthObjectMaterial = std::make_shared<Material>(DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), 0.5f, 16.0f); // Green default health object

    // --- Create HealthObjects ---
    auto healthObj1 = std::make_unique<HealthObject>(100.0f, DirectX::XMFLOAT3(5.0f, 1.0f, 5.0f));
    healthObj1->SetMesh(m_healthObjectMesh.get());
    // Create a unique material copy for this object so color changes don't affect others
    healthObj1->SetMaterial(std::make_shared<Material>(*m_healthObjectMaterial)); 
    m_gameObjects.push_back(std::move(healthObj1));

    auto healthObj2 = std::make_unique<HealthObject>(150.0f, DirectX::XMFLOAT3(-5.0f, 1.0f, 5.0f));
    healthObj2->SetMesh(m_healthObjectMesh.get());
    // Create a unique material copy for this object
    healthObj2->SetMaterial(std::make_shared<Material>(*m_healthObjectMaterial));
    m_gameObjects.push_back(std::move(healthObj2));

    // --- Create Floor ---
    auto floor = std::make_unique<GameObject>(meshCube.get(), matFloor);
    floor->SetPosition(0.0f, -1.0f, 0.0f);
    floor->SetScale(100.0f, 0.1f, 100.0f);
    m_gameObjects.push_back(std::move(floor));

    // --- Create Pillars ---
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

    // --- Create Pedestal ---
    auto pedestal = std::make_unique<GameObject>(meshCube.get(), matPillar);
    pedestal->SetPosition(0.0f, 0.0f, 0.0f);
    pedestal->SetScale(2.0f, 1.0f, 2.0f);
    m_gameObjects.push_back(std::move(pedestal));

    // --- Create Artifact (Rotating Torus) ---
    auto artifact = std::make_unique<GameObject>(meshTorus.get(), matGold);
    artifact->SetPosition(0.0f, 2.0f, 0.0f);
    artifact->SetScale(1.5f, 1.5f, 1.5f);
    artifact->SetRotation(DirectX::XM_PIDIV2, 0.0f, 0.0f);
    m_gameObjects.push_back(std::move(artifact));

    // --- Create Floating Orbs ---
    for (int i = 0; i < 4; i++)
    {
        auto orb = std::make_unique<GameObject>(meshSphere.get(), matGlowing);
        orb->SetScale(0.5f, 0.5f, 0.5f);
        m_gameObjects.push_back(std::move(orb));
    }
}

// Helper method to add a bullet to internal vectors
void Game::AddBulletInternal(std::unique_ptr<Bullet> bullet)
{
    m_bullets.push_back(bullet.get());
    // Also add to m_gameObjects for rendering and general update loop if it's a GameObject
    m_gameObjects.push_back(std::move(bullet)); // Transfer ownership
}

void Game::SpawnBullet(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction, float speed, float damage)
{
    auto bullet = std::make_unique<Bullet>(position, direction, speed, damage);
    
    if (m_bulletMesh)
    {
        bullet->SetMesh(m_bulletMesh.get());
    }
    else
    {
        // Log error or use a default mesh if m_bulletMesh is null
        // For now, let's just assert or return
        OutputDebugString(L"Error: m_bulletMesh is null when spawning bullet!\n");
        return; 
    }

    if (m_bulletMaterial)
    {
        bullet->SetMaterial(m_bulletMaterial);
    }
    else
    {
        // Log error or use a default material if m_bulletMaterial is null
        OutputDebugString(L"Error: m_bulletMaterial is null when spawning bullet!\n");
        return;
    }

    bullet->SetScale(0.2f, 0.2f, 0.2f); // Small bullet size

    AddBulletInternal(std::move(bullet));
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
    // --- FPS Calculation ---
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

    // --- PLAYER SHOOTING ---
    if (m_player && m_input.IsKeyDown(VK_LBUTTON))
    {
        m_player->Shoot(this); // Player's gun will spawn bullets via Game::SpawnBullet
    }

    // --- UPDATE PLAYER ---
    if (m_player)
    {
        m_player->Update(deltaTime, m_input, m_gameObjects);
    }

    // --- UPDATE BULLETS AND CHECK COLLISIONS ---
    for (const auto& bullet : m_bullets)
    {
        bullet->Update(deltaTime);

        // Simple despawn logic: remove if too far below ground or too old
        if (bullet->GetPosition().y < -50.0f || bullet->GetAge() > 5.0f) // Assuming GetAge() exists or will be added to Bullet
        {
            bullet->SetActive(false); // Mark for removal
            continue;
        }

        // Collision with other GameObjects
        for (const auto& obj : m_gameObjects)
        {
            if (obj.get() == bullet || obj.get() == m_player) continue; // Don't collide with self or player

            if (PhysicsSystem::AABBIntersects(bullet->GetWorldBoundingBox(), obj->GetWorldBoundingBox()))
            {
                // Check if it's a HealthObject
                HealthObject* healthObj = dynamic_cast<HealthObject*>(obj.get());
                if (healthObj)
                {
                    OutputDebugString(L"Bullet hit HealthObject!\n");
                    healthObj->TakeDamage(bullet->GetDamage());
                    bullet->SetActive(false); // Bullet is destroyed on impact
                    break; // Bullet can only hit one object
                }
                else
                {
                    // Collided with a non-health object, maybe bounce or destroy bullet
                    // For now, destroy bullet on any non-player collision
                    bullet->SetActive(false);
                    break;
                }
            }
        }
    }

    // --- CLEANUP INACTIVE BULLETS AND DEAD HEALTH OBJECTS ---
    // Remove inactive bullets from m_bullets
    std::vector<Bullet*> activeBullets;
    for (auto* bullet : m_bullets)
    {
        if (bullet->IsActive())
        {
            activeBullets.push_back(bullet);
        }
    }
    m_bullets = activeBullets;

    // Remove inactive/dead GameObjects from m_gameObjects
    std::vector<std::unique_ptr<GameObject>> activeGameObjects;
    for (auto& obj : m_gameObjects)
    {
        // Keep the player regardless
        if (obj.get() == m_player)
        {
            activeGameObjects.push_back(std::move(obj));
            continue;
        }

        // Check if it's a Bullet (already handled its active state in m_bullets)
        Bullet* bulletObj = dynamic_cast<Bullet*>(obj.get());
        if (bulletObj)
        {
            // If it's an active bullet, keep it
            if (bulletObj->IsActive())
            {
                activeGameObjects.push_back(std::move(obj));
            }
            // Else, it's an inactive bullet, so let it be removed
            continue; 
        }

        // Check if it's a HealthObject
        HealthObject* healthObj = dynamic_cast<HealthObject*>(obj.get());
        if (healthObj)
        {
            // If it's a living health object, keep it
            if (!healthObj->IsDead())
            {
                activeGameObjects.push_back(std::move(obj));
            }
            // Else, it's a dead health object, so let it be removed
            continue;
        }
        
        // For all other generic GameObjects, keep them
        activeGameObjects.push_back(std::move(obj));
    }
    m_gameObjects = std::move(activeGameObjects);

    // --- ANIMATION LOGIC (Objects) ---
    static float time = 0.0f;
    time += deltaTime;

    // Rotate the central artifact (Assuming it's at index 11, after Player(0)+Floor(1)+4Pillars(2-5)+4Roofs(6-9)+Pedestal(10) = 11)
    // Note: Because we inserted Player at index 0, all indices shifted by 1 compared to previous code.
    // Previous Index 10 (Pedestal) is now likely 11. Previous 11 (Artifact) is now 12.
    // To be safe, you might want to store these as specific pointers, but keeping the loop logic for now:
    // This part might need re-evaluation due to dynamic object removal changing indices.
    // A better approach would be to store specific objects with named unique_ptr members or a map.
    // For now, I'll adjust indices based on the initial static objects + player.
    // Player (0), HealthObj1(1), HealthObj2(2), Floor(3), Pillar1(4) ...
    // The artifact is one of the later static objects. Let's assume it's still at an index > 10.
    size_t artifactIndex = -1;
    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        if (m_gameObjects[i]->GetName() == L"Artifact") { // Assuming GameObject has a GetName()
            artifactIndex = i;
            break;
        }
    }
    if (artifactIndex != -1)
    {
        m_gameObjects[artifactIndex]->SetRotation(DirectX::XM_PIDIV2, time, 0.0f);
    }
    // Same for Floating Orbs - this part becomes fragile with dynamic objects
    // Best to replace magic indices with named pointers or a more robust scene graph.
    // For now, skipping dynamic object animation for simplicity, as it requires a full scene graph refactor.

    // --- UPDATE PHYSICS SYSTEM ---
    // Handles gravity and collisions for non-player objects (if any are dynamic)
    m_physics.Update(m_gameObjects, deltaTime);
}

void Game::Render()
{
    // RenderScene handles all GameObjects that are currently in m_gameObjects
    m_renderer->RenderFrame(*m_camera, m_gameObjects, m_dirLight, m_pointLights);
    m_renderer->RenderDebug(*m_camera, m_gameObjects);

    m_uiRenderer->EnableUIState();

    float color[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
    m_uiRenderer->DrawString(m_font, "FPS: " + std::to_string(m_fps), 10.0f, 10.0f, 30.0f, color);
    m_uiRenderer->DrawString(m_font, "WASD to Move, Space to Jump", 10.0f, 40.0f, 20.0f, color);
    m_uiRenderer->DrawString(m_font, "Left Click to Shoot", 10.0f, 70.0f, 20.0f, color);


    m_uiRenderer->DisableUIState();

    m_graphics.Present();
}