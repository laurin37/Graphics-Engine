#include "../../include/Application/Scene.h"
#include "../../include/ResourceManagement/AssetManager.h"
#include "../../include/Renderer/Graphics.h"
#include "../../include/UI/UIRenderer.h"
#include "../../include/Renderer/Material.h"
#include "../../include/Renderer/Renderer.h"
#include "../../include/ResourceManagement/TextureLoader.h"
#include "../../include/ResourceManagement/FontLoader.h"
#include "../../include/ResourceManagement/SceneLoader.h"
#include "../../include/Input/Input.h"
#include "../../include/Renderer/PostProcess.h"
#include <format>

Scene::Scene(AssetManager* assetManager, Graphics* graphics)
    : m_assetManager(assetManager), 
      m_graphics(graphics),
      m_dirLight{ {0.5f, -0.7f, 0.5f, 0.0f}, {0.2f, 0.2f, 0.3f, 1.0f} }
{
}

Scene::~Scene() = default;

void Scene::Load()
{
    // Load all meshes
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
    
    // Create materials
    m_matFloor = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.2f, 10.0f, texWood, normWood);
    m_matPillar = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.8f, 32.0f, texMetal, normMetal);
    m_matRoof = std::make_shared<Material>(DirectX::XMFLOAT4(0.8f, 0.1f, 0.1f, 1.0f), 0.8f, 32.0f);
    m_matGold = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.8f, 0.0f, 1.0f), 1.0f, 64.0f);
    m_matGlowing = std::make_shared<Material>(DirectX::XMFLOAT4(2.0f, 2.0f, 2.0f, 1.0f), 1.0f, 256.0f);
    m_matOrbRed = std::make_shared<Material>(DirectX::XMFLOAT4(2.5f, 1.2f, 1.2f, 1.0f), 1.0f, 256.0f);
    m_matOrbGreen = std::make_shared<Material>(DirectX::XMFLOAT4(1.2f, 2.5f, 1.2f, 1.0f), 1.0f, 256.0f);
    m_matOrbBlue = std::make_shared<Material>(DirectX::XMFLOAT4(1.2f, 1.2f, 2.5f, 1.0f), 1.0f, 256.0f);
    m_matOrbOrange = std::make_shared<Material>(DirectX::XMFLOAT4(2.5f, 2.0f, 1.2f, 1.0f), 1.0f, 256.0f);
    
    // Load font
    if (m_graphics) {
        try {
            auto fontData = FontLoader::Load(m_graphics->GetDevice().Get(), m_graphics->GetContext().Get(), 
                                            L"Assets/Textures/bitmap/Minecraft.ttf", L"Minecraft", 32.0f);
            m_font.Initialize(fontData.texture, fontData.glyphs);
        } catch (const std::exception&) {
            auto debugFontTex = TextureLoader::CreateDebugFont(m_graphics->GetDevice().Get(), m_graphics->GetContext().Get());
            m_font.Initialize(debugFontTex);
        }
    }
    
    // Initialize UI
    m_crosshair = std::make_unique<Crosshair>();
    
    // Load scene from JSON
    LoadSceneFromJSON(L"Assets/Scenes/default.json");
}

void Scene::LoadSceneFromJSON(const std::wstring& jsonPath) {
    auto meshLookup = BuildMeshLookup();
    auto materialLookup = BuildMaterialLookup();
    
    SceneLoader::LoadScene(jsonPath, m_ecsComponentManager, meshLookup, materialLookup);
}

std::unordered_map<std::string, Mesh*> Scene::BuildMeshLookup() {
    std::unordered_map<std::string, Mesh*> lookup;
    
    if (m_meshCube) lookup["cube"] = m_meshCube.get();
    if (m_meshCylinder) lookup["cylinder"] = m_meshCylinder.get();
    if (m_meshCone) lookup["cone"] = m_meshCone.get();
    if (m_meshSphere) lookup["sphere"] = m_meshSphere.get();
    if (m_meshTorus) lookup["torus"] = m_meshTorus.get();
    if (m_meshRoom) lookup["room"] = m_meshRoom.get();
    
    return lookup;
}

std::unordered_map<std::string, std::shared_ptr<Material>> Scene::BuildMaterialLookup() {
    std::unordered_map<std::string, std::shared_ptr<Material>> lookup;
    
    if (m_matFloor) lookup["matFloor"] = m_matFloor;
    if (m_matPillar) lookup["matPillar"] = m_matPillar;
    if (m_matRoof) lookup["matRoof"] = m_matRoof;
    if (m_matGold) lookup["matGold"] = m_matGold;
    if (m_matGlowing) lookup["matGlowing"] = m_matGlowing;
    if (m_matOrbRed) lookup["matOrbRed"] = m_matOrbRed;
    if (m_matOrbGreen) lookup["matOrbGreen"] = m_matOrbGreen;
    if (m_matOrbBlue) lookup["matOrbBlue"] = m_matOrbBlue;
    if (m_matOrbOrange) lookup["matOrbOrange"] = m_matOrbOrange;
    
    return lookup;
}

void Scene::Update(float deltaTime, Input& input)
{
    // FPS calculation
    m_frameCount++;
    m_timeAccum += deltaTime;
    if (m_timeAccum >= 1.0f)
    {
        m_fps = m_frameCount;
        m_frameCount = 0;
        m_timeAccum -= 1.0f;
    }

    // Update ECS systems
    m_ecsPlayerMovementSystem.Update(m_ecsComponentManager, input, deltaTime);
    m_ecsPhysicsSystem.Update(m_ecsComponentManager, deltaTime);
    m_ecsMovementSystem.Update(m_ecsComponentManager, deltaTime);
    m_ecsCameraSystem.Update(m_ecsComponentManager);
}

void Scene::Render(Renderer* renderer, UIRenderer* uiRenderer, bool showDebugCollision)
{
    if (!renderer || !uiRenderer) return;

    // Create temporary GameObjects for ECS entities to use existing Renderer
    std::vector<std::unique_ptr<GameObject>> ecsRenderObjects;
    
    // Get all entities with Transform and Render components
    const auto& entities = m_ecsComponentManager.GetEntitiesWithRenderAndTransform();
    
    for (ECS::Entity entity : entities) {
        auto* transform = m_ecsComponentManager.GetTransform(entity);
        auto* render = m_ecsComponentManager.GetRender(entity);
        
        if (transform && render && render->mesh && render->material) {
            auto obj = std::make_unique<GameObject>(render->mesh, render->material);
            obj->SetPosition(transform->position.x, transform->position.y, transform->position.z);
            obj->SetRotation(transform->rotation.x, transform->rotation.y, transform->rotation.z);
            obj->SetScale(transform->scale.x, transform->scale.y, transform->scale.z);
            ecsRenderObjects.push_back(std::move(obj));
        }
    }
    
    // Gather ECS lights
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
    
    // Get ECS camera and create temporary Camera adapter for renderer
    DirectX::XMMATRIX ecsView, ecsProj;
    Camera tempCamera;  // Temporary adapter
    
    if (m_ecsCameraSystem.GetActiveCamera(m_ecsComponentManager, ecsView, ecsProj)) {
        // Find player entity with camera to get position/rotation
        ECS::Entity cameraEntity = m_ecsComponentManager.GetActiveCamera();
        if (auto* transform = m_ecsComponentManager.GetTransform(cameraEntity)) {
            tempCamera.SetPosition(transform->position.x, transform->position.y, transform->position.z);
            tempCamera.SetRotation(transform->rotation.x, transform->rotation.y, transform->rotation.z);
        }
    }
    
    // Render scene
    renderer->RenderFrame(tempCamera, ecsRenderObjects, m_dirLight, ecsLights);
    
    // Render debug collision boxes
    if (showDebugCollision) {
        m_ecsRenderSystem.RenderDebug(m_ecsComponentManager, renderer, tempCamera);
    }
    
    // Render UI
    uiRenderer->EnableUIState();

    if (m_crosshair) {
        m_crosshair->Draw(uiRenderer, m_font, 1280, 720);
    }

    // Render debug UI (can be toggled with F1)
    m_debugUI.Render(
        uiRenderer, 
        m_font, 
        m_fps, 
        renderer->GetPostProcess()->IsBloomEnabled(),
        showDebugCollision,
        m_ecsComponentManager
    );

    uiRenderer->DisableUIState();
}
