#include "include/Scene.h"
#include "include/AssetManager.h"
#include "include/Renderer.h"
#include "include/UIRenderer.h"
#include "include/Material.h"
#include "include/ModelLoader.h"
#include "include/TextureLoader.h"
#include "include/Collision.h"
#include "include/Gun.h"
#include "include/HealthObject.h"
#include "include/Input.h"

Scene::Scene(AssetManager* assetManager, Graphics* graphics)
    : m_assetManager(assetManager), m_graphics(graphics),
    m_dirLight{ {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} }
{
}

Scene::~Scene() = default;

void Scene::Load()
{
    // 1. Camera Setup
    m_camera = std::make_unique<Camera>();
    m_camera->SetPosition(0.0f, 5.0f, -15.0f);
    m_camera->AdjustRotation(0.3f, 0.0f, 0.0f);

    // 2. Setup Lights
    m_dirLight.direction = { 0.5f, -0.7f, 0.5f, 0.0f };
    m_dirLight.color = { 0.2f, 0.2f, 0.3f, 1.0f };

    m_pointLights.resize(MAX_POINT_LIGHTS);
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
    if (m_graphics) {
        auto debugFontTex = TextureLoader::CreateDebugFont(m_graphics->GetDevice().Get(), m_graphics->GetContext().Get());
        m_font.Initialize(debugFontTex);
    }

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
    auto gunMesh = meshCube; 
    auto gunMaterial = matPillar; 

    auto gunPtr = std::make_unique<Gun>(gunMesh.get(), gunMaterial);
    gunPtr->SetPosition(m_player->GetPosition().x + 0.5f, m_player->GetPosition().y + 0.5f, m_player->GetPosition().z + 0.5f);
    gunPtr->SetScale(0.2f, 0.2f, 0.8f); 

    m_player->SetGun(gunPtr.get()); 
    gunPtr->SetOwner(m_player); 
    m_gameObjects.push_back(std::move(gunPtr)); 

    // --- Store Bullet and HealthObject assets ---
    m_bulletMesh = meshSphere; 
    m_bulletMaterial = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f), 1.0f, 64.0f); 

    m_healthObjectMesh = meshCube; 
    m_healthObjectMaterial = std::make_shared<Material>(DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), 0.5f, 16.0f); 

    // --- Create HealthObjects ---
    // Positioned away from pillars (pillars are at ±6, ±6)
    auto healthObj1 = std::make_unique<HealthObject>(100.0f, DirectX::XMFLOAT3(10.0f, 1.0f, 0.0f));
    healthObj1->SetMesh(m_healthObjectMesh.get());
    healthObj1->SetMaterial(std::make_shared<Material>(*m_healthObjectMaterial));
    healthObj1->GenerateCollider(); 
    m_gameObjects.push_back(std::move(healthObj1));

    auto healthObj2 = std::make_unique<HealthObject>(150.0f, DirectX::XMFLOAT3(-10.0f, 1.0f, 0.0f));
    healthObj2->SetMesh(m_healthObjectMesh.get());
    healthObj2->SetMaterial(std::make_shared<Material>(*m_healthObjectMaterial));
    healthObj2->GenerateCollider();
    m_gameObjects.push_back(std::move(healthObj2));

    // --- Create Floor ---
    auto floor = std::make_unique<GameObject>(meshCube.get(), matFloor);
    floor->SetPosition(0.0f, -1.0f, 0.0f);
    floor->SetScale(100.0f, 0.1f, 100.0f);
    floor->GenerateCollider(); // Auto-generate collision from cube mesh
    m_gameObjects.push_back(std::move(floor));

    // --- Create Pillars ---
    float pillarDist = 6.0f;
    float pillarPositions[4][2] = { {pillarDist, pillarDist}, {pillarDist, -pillarDist}, {-pillarDist, pillarDist}, {-pillarDist, -pillarDist} };

    for (int i = 0; i < 4; i++)
    {
        auto pillar = std::make_unique<GameObject>(meshCylinder.get(), matPillar);
        pillar->SetPosition(pillarPositions[i][0], 1.0f, pillarPositions[i][1]);
        pillar->SetScale(1.0f, 2.0f, 1.0f);
        pillar->GenerateCollider(); // Auto-generate collision
        m_gameObjects.push_back(std::move(pillar));

        auto roof = std::make_unique<GameObject>(meshCone.get(), matRoof);
        roof->SetPosition(pillarPositions[i][0], 3.5f, pillarPositions[i][1]);
        roof->SetScale(1.5f, 1.0f, 1.5f);
        roof->GenerateCollider(); // Auto-generate collision
        m_gameObjects.push_back(std::move(roof));
    }

    // --- Create Pedestal ---
    auto pedestal = std::make_unique<GameObject>(meshCube.get(), matPillar);
    pedestal->SetPosition(0.0f, 0.0f, 0.0f);
    pedestal->SetScale(2.0f, 1.0f, 2.0f);
    pedestal->GenerateCollider();
    m_gameObjects.push_back(std::move(pedestal));

    // --- Create Artifact (Rotating Torus) ---
    auto artifact = std::make_unique<GameObject>(meshTorus.get(), matGold);
    artifact->SetPosition(0.0f, 2.0f, 0.0f);
    artifact->SetScale(1.5f, 1.5f, 1.5f);
    artifact->GenerateCollider();
    artifact->SetRotation(DirectX::XM_PIDIV2, 0.0f, 0.0f);
    artifact->SetName(L"Artifact"); // CRITICAL: Name needed for animation lookup
    m_gameObjects.push_back(std::move(artifact));

    // --- Create Floating Orbs ---
    float orbRadius = 3.0f; // Orbit radius around torus
    for (int i = 0; i < 4; i++)
    {
        auto orb = std::make_unique<GameObject>(meshSphere.get(), matGlowing);
        orb->SetScale(0.5f, 0.5f, 0.5f);
        
        // Initial position in circle around torus
        float angle = (DirectX::XM_2PI / 4.0f) * i;
        float x = orbRadius * cosf(angle);
        float z = orbRadius * sinf(angle);
        orb->SetPosition(x, 2.0f, z); // Same height as torus
        orb->SetName(L"Orb"); // Mark as orb for animation
        
        m_gameObjects.push_back(std::move(orb));
    }

    // Create Crosshair
    m_crosshair = std::make_unique<Crosshair>();
}

void Scene::AddBulletInternal(std::unique_ptr<Bullet> bullet)
{
    m_bullets.push_back(bullet.get());
    m_gameObjects.push_back(std::move(bullet)); 
}

void Scene::SpawnBullet(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction, float speed, float damage)
{
    auto bullet = std::make_unique<Bullet>(position, direction, speed, damage);
    
    if (m_bulletMesh) bullet->SetMesh(m_bulletMesh.get());
    if (m_bulletMaterial) bullet->SetMaterial(m_bulletMaterial);

    bullet->SetScale(0.2f, 0.2f, 0.2f); 
    AddBulletInternal(std::move(bullet));
}

void Scene::Update(float deltaTime, Input& input)
{
    // FPS Calc
    m_frameCount++;
    m_timeAccum += deltaTime;
    if (m_timeAccum >= 1.0f)
    {
        m_fps = m_frameCount;
        m_frameCount = 0;
        m_timeAccum -= 1.0f;
    }

    // Player Shooting
    if (m_player && input.IsKeyDown(VK_LBUTTON))
    {
        m_player->Shoot(this); 
    }

    // Update Player
    if (m_player)
    {
        m_player->Update(deltaTime, input, m_gameObjects);
    }

    // Update Bullets
    for (const auto& bullet : m_bullets)
    {
        bullet->Update(deltaTime);

        if (bullet->GetPosition().y < -50.0f || bullet->GetAge() > 5.0f) 
        {
            bullet->SetActive(false); 
            continue;
        }

        for (const auto& obj : m_gameObjects)
        {
            if (obj.get() == bullet || obj.get() == m_player) continue; 
            
            // Skip gun to prevent false hits
            Gun* gun = dynamic_cast<Gun*>(obj.get());
            if (gun) continue; 

            if (PhysicsSystem::AABBIntersects(bullet->GetWorldBoundingBox(), obj->GetWorldBoundingBox()))
            {
                HealthObject* healthObj = dynamic_cast<HealthObject*>(obj.get());
                if (healthObj)
                {
                    OutputDebugString(L"Bullet hit HealthObject!\n");
                    healthObj->TakeDamage(bullet->GetDamage());
                    bullet->SetActive(false); 
                    break; 
                }
                else
                {
                    bullet->SetActive(false);
                    break;
                }
            }
        }
    }

    // Cleanup
    std::vector<Bullet*> activeBullets;
    for (auto* bullet : m_bullets)
    {
        if (bullet->IsActive()) activeBullets.push_back(bullet);
    }
    m_bullets = activeBullets;

    std::vector<std::unique_ptr<GameObject>> activeGameObjects;
    for (auto& obj : m_gameObjects)
    {
        if (obj.get() == m_player)
        {
            activeGameObjects.push_back(std::move(obj));
            continue;
        }

        Bullet* bulletObj = dynamic_cast<Bullet*>(obj.get());
        if (bulletObj)
        {
            if (bulletObj->IsActive()) activeGameObjects.push_back(std::move(obj));
            continue; 
        }

        HealthObject* healthObj = dynamic_cast<HealthObject*>(obj.get());
        if (healthObj)
        {
            if (!healthObj->IsDead()) activeGameObjects.push_back(std::move(obj));
            continue;
        }
        
        activeGameObjects.push_back(std::move(obj));
    }
    m_gameObjects = std::move(activeGameObjects);

    // Animation
    static float time = 0.0f;
    time += deltaTime;

    size_t artifactIndex = -1;
    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        if (m_gameObjects[i]->GetName() == L"Artifact") { 
            artifactIndex = i;
            break;
        }
    }
    if (artifactIndex != -1)
    {
        m_gameObjects[artifactIndex]->SetRotation(DirectX::XM_PIDIV2, time, 0.0f);
    }

    // Animate Orbs (orbit around artifact)
    float orbRadius = 3.0f;
    float orbSpeed = 1.0f; // radians per second
    int orbCount = 0;
    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        if (m_gameObjects[i]->GetName() == L"Orb") {
            float angle = (DirectX::XM_2PI / 4.0f) * orbCount + (time * orbSpeed);
            float x = orbRadius * cosf(angle);
            float z = orbRadius * sinf(angle);
            float y = 2.0f + 0.3f * sinf(time * 2.0f + orbCount); // Gentle up/down bob
            m_gameObjects[i]->SetPosition(x, y, z);
            orbCount++;
        }
    }

    m_physics.Update(m_gameObjects, deltaTime);
}

void Scene::Render(Renderer* renderer, UIRenderer* uiRenderer)
{
    if (!renderer || !uiRenderer) return;

    renderer->RenderFrame(*m_camera, m_gameObjects, m_dirLight, m_pointLights);
    renderer->RenderDebug(*m_camera, m_gameObjects);

    uiRenderer->EnableUIState();

    // UI text disabled for cleaner view
    // float color[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
    // uiRenderer->DrawString(m_font, "WASD to Move, Space to Jump", 10.0f, 40.0f, 20.0f, color);
    // uiRenderer->DrawString(m_font, "Left Click to Shoot", 10.0f, 70.0f, 20.0f, color);

    if (m_crosshair)
    {
        m_crosshair->Draw(uiRenderer, m_font, 1280.0f, 720.0f); 
    }

    uiRenderer->DisableUIState();
}
