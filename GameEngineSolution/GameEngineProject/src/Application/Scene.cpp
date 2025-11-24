#include "../../include/Application/Scene.h"
#include "../../include/ResourceManagement/AssetManager.h"
#include "../../include/Renderer/Graphics.h"
#include "../../include/UI/UIRenderer.h"
#include "../../include/Renderer/Material.h"
#include "../../include/Renderer/Renderer.h"
#include "../../include/ResourceManagement/ModelLoader.h"
#include "../../include/ResourceManagement/TextureLoader.h"
#include "../../include/Physics/Collision.h"
#include "../../include/EntityComponentSystem/Gun.h"
#include "../../include/EntityComponentSystem/HealthObject.h"
#include "../../include/Input/Input.h"
#include "../../include/Renderer/RenderingConstants.h"
#include "../../include/ResourceManagement/FontLoader.h"
#include "../../include/Renderer/PostProcess.h"

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

    // --- ECS Scene Generation ---
    
    // 1. Floor
    ECS::Entity floor = m_ecsComponentManager.CreateEntity();
    ECS::TransformComponent floorTrans;
    floorTrans.position = { 0.0f, -1.0f, 0.0f };
    floorTrans.scale = { 100.0f, 0.1f, 100.0f };
    m_ecsComponentManager.AddTransform(floor, floorTrans);
    ECS::RenderComponent floorRender;
    floorRender.mesh = m_meshCube.get();
    floorRender.material = m_matFloor;
    m_ecsComponentManager.AddRender(floor, floorRender);
    ECS::PhysicsComponent floorPhys;
    floorPhys.useGravity = false; // Static
    m_ecsComponentManager.AddPhysics(floor, floorPhys);
    ECS::ColliderComponent floorCol;
    floorCol.localAABB.extents = { 50.0f, 0.05f, 50.0f };
    m_ecsComponentManager.AddCollider(floor, floorCol);

    // 2. Room
    ECS::Entity room = m_ecsComponentManager.CreateEntity();
    ECS::TransformComponent roomTrans;
    roomTrans.position = { 30.0f, -1.0f, 30.0f };
    m_ecsComponentManager.AddTransform(room, roomTrans);
    ECS::RenderComponent roomRender;
    roomRender.mesh = m_meshRoom.get();
    roomRender.material = m_matGold;
    m_ecsComponentManager.AddRender(room, roomRender);
    ECS::PhysicsComponent roomPhys;
    roomPhys.useGravity = false;
    m_ecsComponentManager.AddPhysics(room, roomPhys);
    ECS::ColliderComponent roomCol; // Simplified collider
    roomCol.localAABB.extents = { 10.0f, 5.0f, 10.0f }; 
    m_ecsComponentManager.AddCollider(room, roomCol);

    // 3. Pillars & Roofs
    float pillarDist = 6.0f;
    float pillarPositions[4][2] = { {pillarDist, pillarDist}, {pillarDist, -pillarDist}, {-pillarDist, pillarDist}, {-pillarDist, -pillarDist} };
    for (int i = 0; i < 4; i++) {
        // Pillar
        ECS::Entity pillar = m_ecsComponentManager.CreateEntity();
        ECS::TransformComponent pilTrans;
        pilTrans.position = { pillarPositions[i][0], 1.0f, pillarPositions[i][1] };
        pilTrans.scale = { 1.0f, 2.0f, 1.0f };
        m_ecsComponentManager.AddTransform(pillar, pilTrans);
        ECS::RenderComponent pilRender;
        pilRender.mesh = m_meshCylinder.get();
        pilRender.material = m_matPillar;
        m_ecsComponentManager.AddRender(pillar, pilRender);
        ECS::PhysicsComponent pilPhys;
        pilPhys.useGravity = false;
        m_ecsComponentManager.AddPhysics(pillar, pilPhys);
        ECS::ColliderComponent pilCol;
        pilCol.localAABB.extents = { 0.5f, 1.0f, 0.5f };
        m_ecsComponentManager.AddCollider(pillar, pilCol);

        // Roof
        ECS::Entity roof = m_ecsComponentManager.CreateEntity();
        ECS::TransformComponent roofTrans;
        roofTrans.position = { pillarPositions[i][0], 3.5f, pillarPositions[i][1] };
        roofTrans.scale = { 1.5f, 1.0f, 1.5f };
        m_ecsComponentManager.AddTransform(roof, roofTrans);
        ECS::RenderComponent roofRender;
        roofRender.mesh = m_meshCone.get();
        roofRender.material = m_matRoof;
        m_ecsComponentManager.AddRender(roof, roofRender);
        ECS::PhysicsComponent roofPhys;
        roofPhys.useGravity = false;
        m_ecsComponentManager.AddPhysics(roof, roofPhys);
    }

    // 4. Pedestal
    ECS::Entity pedestal = m_ecsComponentManager.CreateEntity();
    ECS::TransformComponent pedTrans;
    pedTrans.position = { 0.0f, 0.0f, 0.0f };
    pedTrans.scale = { 2.0f, 1.0f, 2.0f };
    m_ecsComponentManager.AddTransform(pedestal, pedTrans);
    ECS::RenderComponent pedRender;
    pedRender.mesh = m_meshCube.get();
    pedRender.material = m_matPillar;
    m_ecsComponentManager.AddRender(pedestal, pedRender);
    ECS::PhysicsComponent pedPhys;
    pedPhys.useGravity = false;
    m_ecsComponentManager.AddPhysics(pedestal, pedPhys);
    ECS::ColliderComponent pedCol;
    pedCol.localAABB.extents = { 1.0f, 0.5f, 1.0f };
    m_ecsComponentManager.AddCollider(pedestal, pedCol);

    // 5. Artifact (Rotating)
    ECS::Entity artifact = m_ecsComponentManager.CreateEntity();
    ECS::TransformComponent artTrans;
    artTrans.position = { 0.0f, 2.0f, 0.0f };
    artTrans.scale = { 1.5f, 1.5f, 1.5f };
    artTrans.rotation = { DirectX::XM_PIDIV2, 0.0f, 0.0f }; // Initial rotation
    m_ecsComponentManager.AddTransform(artifact, artTrans);
    ECS::RenderComponent artRender;
    artRender.mesh = m_meshTorus.get();
    artRender.material = m_matGold;
    m_ecsComponentManager.AddRender(artifact, artRender);
    ECS::PhysicsComponent artPhys;
    artPhys.useGravity = false;
    m_ecsComponentManager.AddPhysics(artifact, artPhys);
    ECS::RotateComponent artRot;
    artRot.axis = { 0.0f, 1.0f, 0.0f }; // Rotate around Y
    artRot.speed = 1.0f;
    m_ecsComponentManager.AddRotate(artifact, artRot);

    // 6. Orbs (Orbiting + Lights)
    float orbRadius = 3.0f;
    std::shared_ptr<Material> orbMaterials[4] = { m_matOrbRed, m_matOrbGreen, m_matOrbBlue, m_matOrbOrange };
    DirectX::XMFLOAT4 orbColors[4] = { 
        {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, 
        {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.5f, 0.0f, 1.0f} 
    };

    for (int i = 0; i < 4; i++) {
        ECS::Entity orb = m_ecsComponentManager.CreateEntity();
        ECS::TransformComponent orbTrans;
        orbTrans.scale = { 0.5f, 0.5f, 0.5f };
        // Position set by Orbit system, but init here
        float angle = (DirectX::XM_2PI / 4.0f) * i;
        orbTrans.position = { orbRadius * cosf(angle), 2.0f, orbRadius * sinf(angle) };
        m_ecsComponentManager.AddTransform(orb, orbTrans);
        
        ECS::RenderComponent orbRender;
        orbRender.mesh = m_meshSphere.get();
        orbRender.material = orbMaterials[i];
        m_ecsComponentManager.AddRender(orb, orbRender);
        
        ECS::PhysicsComponent orbPhys;
        orbPhys.useGravity = false;
        m_ecsComponentManager.AddPhysics(orb, orbPhys);
        
        ECS::OrbitComponent orbOrbit;
        orbOrbit.center = { 0.0f, 2.0f, 0.0f };
        orbOrbit.radius = orbRadius;
        orbOrbit.speed = 0.5f;
        orbOrbit.angle = angle;
        m_ecsComponentManager.AddOrbit(orb, orbOrbit);
        
        ECS::LightComponent orbLight;
        orbLight.color = orbColors[i];
        orbLight.intensity = 1.0f;
        orbLight.range = 15.0f;
        m_ecsComponentManager.AddLight(orb, orbLight);
    }
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

    // --- Create cube ---
    auto cube = std::make_unique<GameObject>(m_meshCube.get(), m_matFloor);
    cube->SetPosition(-30.0f, 6.0f, -30.0f);
    cube->SetScale(1.0f, 1.0f, 1.0f);
    cube->GenerateCollider();
    m_gameObjects.push_back(std::move(cube));

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
    // FPS Calc (always run regardless of ECS mode)
    m_frameCount++;
    m_timeAccum += deltaTime;
    if (m_timeAccum >= 1.0f)
    {
        m_fps = m_frameCount;
        LOG_INFO(std::format("FPS: {}", m_fps));
        m_frameCount = 0;
        m_timeAccum -= 1.0f;
    }

    if (m_useECS) {
        // ECS Mode: Update physics system + free camera
        m_ecsPhysicsSystem.Update(m_ecsComponentManager, deltaTime);
        m_ecsMovementSystem.Update(m_ecsComponentManager, deltaTime);
        
        // Free camera controls (WASD + Mouse)
        const float CAMERA_SPEED = 5.0f;
        const float MOUSE_SENSITIVITY = 0.001f;
        
        // Mouse look
        int dx = input.GetMouseDeltaX();
        int dy = input.GetMouseDeltaY();
        m_camera->AdjustRotation(dy * MOUSE_SENSITIVITY, dx * MOUSE_SENSITIVITY, 0.0f);
        
        // WASD movement
        DirectX::XMVECTOR forward = m_camera->GetForward();
        DirectX::XMVECTOR right = m_camera->GetRight();
        DirectX::XMFLOAT3 camPos = m_camera->GetPositionFloat3();
        
        DirectX::XMFLOAT3 fwd, rgt;
        DirectX::XMStoreFloat3(&fwd, forward);
        DirectX::XMStoreFloat3(&rgt, right);
        
        if (input.IsKeyDown('W')) {
            camPos.x += fwd.x * CAMERA_SPEED * deltaTime;
            camPos.y += fwd.y * CAMERA_SPEED * deltaTime;
            camPos.z += fwd.z * CAMERA_SPEED * deltaTime;
        }
        if (input.IsKeyDown('S')) {
            camPos.x -= fwd.x * CAMERA_SPEED * deltaTime;
            camPos.y -= fwd.y * CAMERA_SPEED * deltaTime;
            camPos.z -= fwd.z * CAMERA_SPEED * deltaTime;
        }
        if (input.IsKeyDown('A')) {
            camPos.x -= rgt.x * CAMERA_SPEED * deltaTime;
            camPos.y -= rgt.y * CAMERA_SPEED * deltaTime;
            camPos.z -= rgt.z * CAMERA_SPEED * deltaTime;
        }
        if (input.IsKeyDown('D')) {
            camPos.x += rgt.x * CAMERA_SPEED * deltaTime;
            camPos.y += rgt.y * CAMERA_SPEED * deltaTime;
            camPos.z += rgt.z * CAMERA_SPEED * deltaTime;
        }
        
        // Up/Down with Space/Ctrl
        if (input.IsKeyDown(VK_SPACE)) {
            camPos.y += CAMERA_SPEED * deltaTime;
        }
        if (input.IsKeyDown(VK_CONTROL)) {
            camPos.y -= CAMERA_SPEED * deltaTime;
        }
        
        m_camera->SetPosition(camPos.x, camPos.y, camPos.z);
    } else 
    {
        // GameObject Mode: Update player, bullets, animations, etc.
        
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
    }
}

void Scene::Render(Renderer* renderer, UIRenderer* uiRenderer, bool showDebugCollision)
{
    if (!renderer || !uiRenderer) return;

    if (m_useECS) {
        // ECS Mode: Hybrid Rendering
        // Create temporary GameObjects for ECS entities to use existing Renderer
        std::vector<std::unique_ptr<GameObject>> ecsRenderObjects;
        
        // Get all entities with Transform and Render components
        const auto& entities = m_ecsComponentManager.GetEntitiesWithRenderAndTransform();
        
        for (ECS::Entity entity : entities) {
            auto* transform = m_ecsComponentManager.GetTransform(entity);
            auto* render = m_ecsComponentManager.GetRender(entity);
            
            if (transform && render && render->mesh && render->material) {
                // Create temp GameObject wrapper
                auto obj = std::make_unique<GameObject>(render->mesh, render->material);
                
                // Sync Transform
                obj->SetPosition(transform->position.x, transform->position.y, transform->position.z);
                obj->SetRotation(transform->rotation.x, transform->rotation.y, transform->rotation.z);
                obj->SetScale(transform->scale.x, transform->scale.y, transform->scale.z);
                
                ecsRenderObjects.push_back(std::move(obj));
            }
        }
        
        // Render the temporary objects
        
        // Gather ECS Lights
        std::vector<PointLight> ecsLights;
        const auto& lightEntities = m_ecsComponentManager.GetEntitiesWithLight();
        for (ECS::Entity entity : lightEntities) {
            auto* light = m_ecsComponentManager.GetLight(entity);
            auto* transform = m_ecsComponentManager.GetTransform(entity);
            
            if (light && transform && light->enabled) {
                PointLight pl;
                pl.position = DirectX::XMFLOAT4(transform->position.x, transform->position.y, transform->position.z, light->range);
                pl.color = light->color;
                ecsLights.push_back(pl);
            }
        }
        
        renderer->RenderFrame(*m_camera, ecsRenderObjects, m_dirLight, ecsLights);
        
    } else {
        // GameObject Mode: Render normal scene
        renderer->RenderFrame(*m_camera, m_gameObjects, m_dirLight, m_pointLights);
        
        if (showDebugCollision) {
            renderer->RenderDebug(*m_camera, m_gameObjects);
        }
    }
    
    // Render UI (always, regardless of mode)
    uiRenderer->EnableUIState();

    if (m_crosshair)
    {
        m_crosshair->Draw(uiRenderer, m_font, RenderingConstants::DEFAULT_SCREEN_WIDTH, RenderingConstants::DEFAULT_SCREEN_HEIGHT);
    }

    // Draw FPS
    std::string fpsString = "FPS: " + std::to_string(m_fps);
    float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    uiRenderer->DrawString(m_font, fpsString, 10.0f, 10.0f, 24.0f, green);
    
    // Draw Bloom status
    std::string bloomStatus = renderer->GetPostProcess()->IsBloomEnabled() ? "[B] Bloom: ON" : "[B] Bloom: OFF";
    float yellow[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
    uiRenderer->DrawString(m_font, bloomStatus, 10.0f, 40.0f, 24.0f, yellow);
    
    // Draw Debug Collision status
    std::string debugStatus = showDebugCollision ? "[H] Debug: ON" : "[H] Debug: OFF";
    float cyan[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
    uiRenderer->DrawString(m_font, debugStatus, 10.0f, 70.0f, 24.0f, cyan);
    
    // Draw ECS status
    std::string ecsStatus = m_useECS ? "[E] ECS: ON" : "[E] ECS: OFF";
    float red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    uiRenderer->DrawString(m_font, ecsStatus, 10.0f, 100.0f, 24.0f, red);

    // Draw ECS debug info when enabled
    if (m_useECS) {
        size_t entityCount = m_ecsComponentManager.GetEntityCount();
        std::string entityInfo = "ECS Entities: " + std::to_string(entityCount);
        float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        uiRenderer->DrawString(m_font, entityInfo, 10.0f, 130.0f, 20.0f, white);
        
        // Show position of first entity with transform
        auto entities = m_ecsComponentManager.GetEntitiesWithTransform();
        if (!entities.empty()) {
            ECS::TransformComponent* trans = m_ecsComponentManager.GetTransform(entities[0]);
            if (trans) {
                char posBuffer[128];
                snprintf(posBuffer, sizeof(posBuffer), "Entity[0] Pos: (%.1f, %.1f, %.1f)", 
                    trans->position.x, trans->position.y, trans->position.z);
                uiRenderer->DrawString(m_font, posBuffer, 10.0f, 155.0f, 18.0f, white);
            }
        }
    }

    uiRenderer->DisableUIState();
}
