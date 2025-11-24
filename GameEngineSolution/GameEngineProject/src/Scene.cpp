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
#include "include/RenderingConstants.h"
#include "include/FontLoader.h"
#include "include/PostProcess.h"

Scene::Scene(AssetManager* assetManager, Graphics* graphics)
    : m_assetManager(assetManager), m_graphics(graphics),
    m_dirLight{ {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} }
{
}

Scene::~Scene() = default;

void Scene::Load()
{
    SetupCamera();
    SetupLighting();
    LoadAssets();
    CreateMaterials();
    SpawnSceneObjects();
    InitializeUI();
}

void Scene::SetupCamera()
{
    m_camera = std::make_unique<Camera>();
    m_camera->SetPosition(0.0f, 5.0f, -15.0f);
    m_camera->AdjustRotation(0.3f, 0.0f, 0.0f);
}

void Scene::SetupLighting()
{
    m_dirLight.direction = { 0.5f, -0.7f, 0.5f, 0.0f };
    m_dirLight.color = { 0.2f, 0.2f, 0.3f, 1.0f };

    m_pointLights.resize(MAX_POINT_LIGHTS);
    // Glowing orb lights: {position(x,y,z,range), color(r,g,b,intensity), attenuation(const,linear,quad,pad)}
    m_pointLights[0] = { {0.0f, 2.0f, 0.0f, 15.0f}, {1.0f, 0.5f, 0.5f, 2.5f}, {0.5f, 0.05f, 0.005f, 0.0f} };  // Red-ish
    m_pointLights[1] = { {0.0f, 2.0f, 0.0f, 15.0f}, {0.5f, 1.0f, 0.5f, 2.5f}, {0.5f, 0.05f, 0.005f, 0.0f} };  // Green-ish
    m_pointLights[2] = { {0.0f, 2.0f, 0.0f, 15.0f}, {0.5f, 0.5f, 1.0f, 2.5f}, {0.5f, 0.05f, 0.005f, 0.0f} };  // Blue-ish
    m_pointLights[3] = { {0.0f, 2.0f, 0.0f, 15.0f}, {1.0f, 0.8f, 0.5f, 2.5f}, {0.5f, 0.05f, 0.005f, 0.0f} };  // Orange-ish
}

void Scene::LoadAssets()
{
    // Load meshes
    m_meshCube = m_assetManager->LoadMesh("Assets/Models/basic/cube.obj");
    m_meshCylinder = m_assetManager->LoadMesh("Assets/Models/basic/cylinder.obj");
    m_meshCone = m_assetManager->LoadMesh("Assets/Models/basic/cone.obj");
    m_meshSphere = m_assetManager->LoadMesh("Assets/Models/basic/sphere.obj");
    m_meshTorus = m_assetManager->LoadMesh("Assets/Models/basic/torus.obj");
    m_meshRoom = m_assetManager->LoadMesh("Assets/Models/room.obj");

    // Load textures
    auto texWood = m_assetManager->LoadTexture(L"Assets/Textures/pine_bark_diff_4k.jpg");
    auto normWood = m_assetManager->LoadTexture(L"Assets/Textures/pine_bark_disp_4k.png");
    auto texMetal = m_assetManager->LoadTexture(L"Assets/Textures/blue_metal_plate_diff_4k.jpg");
    auto normMetal = m_assetManager->LoadTexture(L"Assets/Textures/blue_metal_plate_disp_4k.png");

    // Generate debug font
    // Generate debug font or load custom font
    if (m_graphics) {
        try {
            // Try to load custom TTF font
            // Note: "Minecraft" is the likely internal face name for Minecraft.ttf
            auto fontData = FontLoader::Load(m_graphics->GetDevice().Get(), m_graphics->GetContext().Get(), L"Assets/Textures/bitmap/Minecraft.ttf", L"Minecraft", 32.0f);
            m_font.Initialize(fontData.texture, fontData.glyphs);
            LOG_INFO("Loaded custom font: Minecraft.ttf");
        }
        catch (const std::exception& e) {
            LOG_WARNING(std::string("Failed to load custom font: ") + e.what());
            
            // Fallback to generated debug font
            auto debugFontTex = TextureLoader::CreateDebugFont(m_graphics->GetDevice().Get(), m_graphics->GetContext().Get());
            m_font.Initialize(debugFontTex); // No glyphs = monospace fallback
            LOG_INFO("Loaded default debug font");
        }
    }
}

void Scene::CreateMaterials()
{
    // Load textures again temporarily (will be optimized with resource caching later)
    auto texWood = m_assetManager->LoadTexture(L"Assets/Textures/pine_bark_diff_4k.jpg");
    auto normWood = m_assetManager->LoadTexture(L"Assets/Textures/pine_bark_disp_4k.png");
    auto texMetal = m_assetManager->LoadTexture(L"Assets/Textures/blue_metal_plate_diff_4k.jpg");
    auto normMetal = m_assetManager->LoadTexture(L"Assets/Textures/blue_metal_plate_disp_4k.png");
    
    m_matFloor = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.2f, 10.0f, texWood, normWood);
    m_matPillar = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.8f, 32.0f, texMetal, normMetal);
    m_matRoof = std::make_shared<Material>(DirectX::XMFLOAT4(0.8f, 0.1f, 0.1f, 1.0f), 0.8f, 32.0f);
    m_matGold = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.8f, 0.0f, 1.0f), 1.0f, 64.0f);
    m_matGlowing = std::make_shared<Material>(DirectX::XMFLOAT4(2.0f, 2.0f, 2.0f, 1.0f), 1.0f, 256.0f); // Bright white emissive (specPower>100 triggers emit)
    
    // Create colored emissive materials for orbs (matching point light colors)
    m_matOrbRed = std::make_shared<Material>(DirectX::XMFLOAT4(2.5f, 1.2f, 1.2f, 1.0f), 1.0f, 256.0f);    // Bright red
    m_matOrbGreen = std::make_shared<Material>(DirectX::XMFLOAT4(1.2f, 2.5f, 1.2f, 1.0f), 1.0f, 256.0f);  // Bright green
    m_matOrbBlue = std::make_shared<Material>(DirectX::XMFLOAT4(1.2f, 1.2f, 2.5f, 1.0f), 1.0f, 256.0f);   // Bright blue
    m_matOrbOrange = std::make_shared<Material>(DirectX::XMFLOAT4(2.5f, 2.0f, 1.2f, 1.0f), 1.0f, 256.0f); // Bright orange
    
    // Store bullet and health object materials
    m_bulletMesh = m_meshSphere;
    m_bulletMaterial = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f), 1.0f, 64.0f);
    m_healthObjectMesh = m_meshCube;
    m_healthObjectMaterial = std::make_shared<Material>(DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), 0.5f, 16.0f);
}

void Scene::SpawnSceneObjects()
{
    // --- Create Player ---
    auto playerPtr = std::make_unique<Player>(m_meshCylinder.get(), m_matPillar, m_camera.get());
    playerPtr->SetPosition(0.0f, 5.0f, -5.0f);
    m_player = playerPtr.get();
    m_gameObjects.push_back(std::move(playerPtr));

    // --- Create Player's Gun ---
    auto gunPtr = std::make_unique<Gun>(m_meshCube.get(), m_matPillar);
    gunPtr->SetPosition(m_player->GetPosition().x + 0.5f, m_player->GetPosition().y + 0.5f, m_player->GetPosition().z + 0.5f);
    gunPtr->SetScale(0.2f, 0.2f, 0.8f);
    
    m_player->SetGun(gunPtr.get()); 
    gunPtr->SetOwner(m_player); 
    m_gameObjects.push_back(std::move(gunPtr)); 

    // --- Create HealthObjects ---
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
    auto floor = std::make_unique<GameObject>(m_meshCube.get(), m_matFloor);
    floor->SetPosition(0.0f, -1.0f, 0.0f);
    floor->SetScale(100.0f, 0.1f, 100.0f);
    floor->GenerateCollider();
    m_gameObjects.push_back(std::move(floor));

    // --- Create Room ---
    auto room = std::make_unique<GameObject>(m_meshRoom.get(), m_matGold);
    room->SetPosition(30.0f, -1.0f, 30.0f);
    room->SetScale(1.0f, 1.0f, 1.0f);
    room->GenerateCollider();
    m_gameObjects.push_back(std::move(room));

    // --- Create Pillars ---
    float pillarDist = 6.0f;
    float pillarPositions[4][2] = { {pillarDist, pillarDist}, {pillarDist, -pillarDist}, {-pillarDist, pillarDist}, {-pillarDist, -pillarDist} };

    for (int i = 0; i < 4; i++)
    {
        auto pillar = std::make_unique<GameObject>(m_meshCylinder.get(), m_matPillar);
        pillar->SetPosition(pillarPositions[i][0], 1.0f, pillarPositions[i][1]);
        pillar->SetScale(1.0f, 2.0f, 1.0f);
        pillar->GenerateCollider();
        m_gameObjects.push_back(std::move(pillar));

        auto roof = std::make_unique<GameObject>(m_meshCone.get(), m_matRoof);
        roof->SetPosition(pillarPositions[i][0], 3.5f, pillarPositions[i][1]);
        roof->SetScale(1.5f, 1.0f, 1.5f);
        roof->GenerateCollider();
        m_gameObjects.push_back(std::move(roof));
    }

    // --- Create Pedestal ---
    auto pedestal = std::make_unique<GameObject>(m_meshCube.get(), m_matPillar);
    pedestal->SetPosition(0.0f, 0.0f, 0.0f);
    pedestal->SetScale(2.0f, 1.0f, 2.0f);
    pedestal->GenerateCollider();
    m_gameObjects.push_back(std::move(pedestal));

    // --- Create Artifact (Rotating Torus) ---
    auto artifact = std::make_unique<GameObject>(m_meshTorus.get(), m_matGold);
    artifact->SetPosition(0.0f, 2.0f, 0.0f);
    artifact->SetScale(1.5f, 1.5f, 1.5f);
    artifact->GenerateCollider();
    artifact->SetRotation(DirectX::XM_PIDIV2, 0.0f, 0.0f);
    artifact->SetName(L"Artifact");
    m_gameObjects.push_back(std::move(artifact));

    // --- Create Floating Orbs ---
    float orbRadius = 3.0f;
    std::shared_ptr<Material> orbMaterials[4] = { m_matOrbRed, m_matOrbGreen, m_matOrbBlue, m_matOrbOrange };
    
    for (int i = 0; i < 4; i++)
    {
        auto orb = std::make_unique<GameObject>(m_meshSphere.get(), orbMaterials[i]);
        orb->SetScale(0.5f, 0.5f, 0.5f);
        
        float angle = (DirectX::XM_2PI / 4.0f) * i;
        float x = orbRadius * cosf(angle);
        float z = orbRadius * sinf(angle);
        orb->SetPosition(x, 2.0f, z);
        orb->SetName(L"Orb");
        orb->GenerateCollider();  // Add hitboxes to orbs
        
        m_gameObjects.push_back(std::move(orb));
    }
}

void Scene::InitializeUI()
{
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

    bullet->SetScale(0.1f, 0.1f, 0.1f);  // Smaller, more realistic bullets
    bullet->GenerateCollider(ColliderType::Sphere); // Add collider entity
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
        LOG_INFO(std::format("FPS: {}", m_fps));
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
                    LOG_DEBUG("Bullet hit HealthObject!");
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

    // Animate orbs orbiting the artifact + sync lights
    DirectX::XMFLOAT3 artifactPos = (artifactIndex != -1) ? 
        m_gameObjects[artifactIndex]->GetPosition() : DirectX::XMFLOAT3(0.0f, 2.0f, 0.0f);
    
    float orbRadius = 3.0f;
    int orbIndex = 0;
    for (size_t i = 0; i < m_gameObjects.size(); ++i)
    {
        if (m_gameObjects[i]->GetName() == L"Orb" && orbIndex < 4)
        {
            float angle = (time * 0.5f) + (orbIndex * DirectX::XM_PIDIV2);
            
            float x = artifactPos.x + cosf(angle) * orbRadius;
            float z = artifactPos.z + sinf(angle) * orbRadius;
            float y = artifactPos.y + sinf(time * 2.0f + orbIndex) * 0.3f;
            
            m_gameObjects[i]->SetPosition(x, y, z);
            
            // Sync point light position (preserve range in w component)
            m_pointLights[orbIndex].position = DirectX::XMFLOAT4(x, y, z, 15.0f);
            
            orbIndex++;
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

    if (m_crosshair)
    {
        m_crosshair->Draw(uiRenderer, m_font, RenderingConstants::DEFAULT_SCREEN_WIDTH, RenderingConstants::DEFAULT_SCREEN_HEIGHT);
    }

    // Draw FPS
    std::string fpsString = "FPS: " + std::to_string(m_fps);
    float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    uiRenderer->DrawString(m_font, fpsString, 10.0f, 10.0f, 32.0f, green);
    
    // Draw Bloom status
    std::string bloomStatus = renderer->GetPostProcess()->IsBloomEnabled() ? "[B] Bloom: ON" : "[B] Bloom: OFF";
    float yellow[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
    uiRenderer->DrawString(m_font, bloomStatus, 10.0f, 50.0f, 24.0f, yellow);

    uiRenderer->DisableUIState();
}
