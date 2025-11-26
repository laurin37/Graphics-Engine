#include "Application/Scene.h"

#include "ResourceManagement/AssetManager.h"
#include "Renderer/Graphics.h"
#include "UI/UIRenderer.h"
#include "Renderer/Material.h"
#include "Renderer/Renderer.h"
#include "ResourceManagement/TextureLoader.h"
#include "ResourceManagement/FontLoader.h"
#include "ResourceManagement/SceneLoader.h"
#include "Input/Input.h"
#include "Renderer/PostProcess.h"
#include "ECS/Systems/ECSPhysicsSystem.h"
#include "ECS/Systems/ECSRenderSystem.h"
#include "ECS/Systems/ECSMovementSystem.h"
#include "Systems/PlayerMovementSystem.h"
#include "ECS/Systems/CameraSystem.h"
#include "Systems/HealthSystem.h"
#include "Systems/WeaponSystem.h"
#include "Systems/ProjectileSystem.h"
#include "Events/Event.h"
#include "Events/EventBus.h"
#include "Events/InputEvents.h"

#include <format>
#include "Config/GameConfig.h"
#include <cmath>

Scene::Scene(AssetManager* assetManager, Graphics* graphics, Input* input, EventBus* eventBus)
    : m_assetManager(assetManager), 
      m_graphics(graphics),
      m_input(input),
      m_eventBus(eventBus),
      m_dirLight{ {0.5f, -0.7f, 0.5f, 0.0f}, {0.2f, 0.2f, 0.3f, 1.0f} }
{
    // Initialize Systems
    // Note: Order matters for Update() calls in SystemManager
    
    // 1. Core Systems
    m_ecsPhysicsSystem = m_systemManager.AddSystem<ECS::PhysicsSystem>(m_ecsComponentManager);
    m_ecsMovementSystem = m_systemManager.AddSystem<ECS::MovementSystem>(m_ecsComponentManager);
    m_ecsCameraSystem = m_systemManager.AddSystem<ECS::CameraSystem>(m_ecsComponentManager);
    
    // 2. Gameplay Systems (depend on Input)
    if (m_input) {
        m_ecsPlayerMovementSystem = m_systemManager.AddSystem<ECS::PlayerMovementSystem>(m_ecsComponentManager, *m_input);
        m_weaponSystem = m_systemManager.AddSystem<WeaponSystem>(m_ecsComponentManager, *m_input);
    }
    m_projectileSystem = m_systemManager.AddSystem<ProjectileSystem>(m_ecsComponentManager);
    m_healthSystem = m_systemManager.AddSystem<HealthSystem>(m_ecsComponentManager);
    
    // 3. Rendering System (needs to be updated manually or last)
    m_ecsRenderSystem = m_systemManager.AddSystem<ECS::RenderSystem>(m_ecsComponentManager);

    // Subscribe systems to EventBus
    if (m_eventBus && m_ecsPlayerMovementSystem) 
    {
        auto id = m_eventBus->Subscribe(EventType::KeyPressed, [this](Event& e) 
        {
            m_ecsPlayerMovementSystem->OnEvent(e);
        }, EventPriority::Normal);
        m_eventSubscriptions.push_back(id);
    }

    // Initialize UI
    m_crosshair = std::make_unique<Crosshair>();
    
    // Load() is called explicitly by Game class
}

Scene::~Scene() 
{
    // Unsubscribe from all events
    if (m_eventBus) {
        for (auto id : m_eventSubscriptions) {
            m_eventBus->Unsubscribe(EventType::KeyPressed, id);
        }
    }
}

void Scene::Load()
{

// ... (inside Load)

    // Load scene from JSON
    LoadSceneFromJSON(Config::Paths::DefaultScene);

    // Load Font
    try {
        FontData fontData = FontLoader::Load(
            m_graphics->GetDevice().Get(), 
            m_graphics->GetContext().Get(), 
            Config::Paths::DefaultFont, 
            Config::UI::FontName,
            Config::UI::FontSize
        );
        m_font.Initialize(fontData.texture, fontData.glyphs);
    } catch (const std::exception&) {
        // Log error but don't crash if font fails (UI will just be blocks)
        // LOG_ERROR(e.what()); 
    }

    // Setup Weapon System assets (special case for now)
    if (m_weaponSystem && m_assetManager) {
        auto sphereMesh = m_assetManager->LoadMesh(Config::Paths::DefaultProjectileMesh);
        
        auto projectileMat = std::make_shared<Material>();
        projectileMat->SetColor({ 1.0f, 0.2f, 0.2f, 1.0f }); // Reddish
        projectileMat->SetSpecular(0.5f);
        projectileMat->SetShininess(32.0f);
        
        m_weaponSystem->SetProjectileAssets(sphereMesh.get(), projectileMat);
    }

    RebuildRenderCache();
}

void Scene::LoadSceneFromJSON(const std::wstring& jsonPath) {
    if (m_assetManager) {
        SceneLoader::LoadScene(jsonPath, m_ecsComponentManager, m_assetManager);
    }
}

void Scene::Update(float deltaTime)
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

    // Update ECS systems via SystemManager
    m_systemManager.Update(deltaTime);
}

void Scene::Render(Renderer* renderer, UIRenderer* uiRenderer, bool showDebugCollision)
{
    if (!renderer || !uiRenderer) return;

    UpdateRenderCache();

    std::vector<const Renderer::RenderInstance*> renderInstanceViews;
    renderInstanceViews.reserve(m_renderCache.size());
    for (auto& entry : m_renderCache)
    {
        renderInstanceViews.push_back(&entry.instance);
    }
    
    // Gather ECS lights
    std::vector<PointLight> ecsLights;
    GatherLights(ecsLights);
    
    // Get ECS camera and create temporary Camera adapter for renderer
    DirectX::XMMATRIX ecsView, ecsProj;
    Camera tempCamera;  // Temporary adapter
    
    if (SetupCamera(tempCamera, ecsView, ecsProj)) {
        // Render scene
        renderer->RenderFrame(tempCamera, renderInstanceViews, m_dirLight, ecsLights);
        
        // Render debug collision boxes
        if (showDebugCollision) {
            m_ecsRenderSystem->RenderDebug(renderer, tempCamera);
        }
    }
    
    // Render UI
    RenderUI(renderer, uiRenderer, showDebugCollision);
}

void Scene::RebuildRenderCache()
{
    m_renderCache.clear();
    m_entityToRenderCacheIndex.clear();

    auto renderArray = m_ecsComponentManager.GetComponentArray<ECS::RenderComponent>();
    auto& renderVec = renderArray->GetComponentArray();
    
    m_renderCache.reserve(renderVec.size());

    for (size_t i = 0; i < renderVec.size(); ++i) {
        ECS::Entity entity = renderArray->GetEntityAtIndex(i);
        ECS::RenderComponent& render = renderVec[i];
        
        if (!render.mesh || !render.material) continue;
        
        if (!m_ecsComponentManager.HasComponent<ECS::TransformComponent>(entity)) continue;
        ECS::TransformComponent& transform = m_ecsComponentManager.GetComponent<ECS::TransformComponent>(entity);
        
        CreateRenderCacheEntry(entity, &transform, &render);
    }
}

void Scene::UpdateRenderCache()
{
    for (size_t i = 0; i < m_renderCache.size();)
    {
        ECS::Entity entity = m_renderCache[i].entity;
        
        // Check if components still exist
        if (!m_ecsComponentManager.HasComponent<ECS::TransformComponent>(entity) ||
            !m_ecsComponentManager.HasComponent<ECS::RenderComponent>(entity))
        {
            RemoveRenderCacheEntry(i);
            continue;
        }
        
        auto& transform = m_ecsComponentManager.GetComponent<ECS::TransformComponent>(entity);
        auto& render = m_ecsComponentManager.GetComponent<ECS::RenderComponent>(entity);

        if (!render.mesh || !render.material)
        {
            RemoveRenderCacheEntry(i);
            continue;
        }

        bool transformChanged =
            transform.position.x != m_renderCache[i].lastPosition.x ||
            transform.position.y != m_renderCache[i].lastPosition.y ||
            transform.position.z != m_renderCache[i].lastPosition.z ||
            transform.rotation.x != m_renderCache[i].lastRotation.x ||
            transform.rotation.y != m_renderCache[i].lastRotation.y ||
            transform.rotation.z != m_renderCache[i].lastRotation.z ||
            transform.scale.x != m_renderCache[i].lastScale.x ||
            transform.scale.y != m_renderCache[i].lastScale.y ||
            transform.scale.z != m_renderCache[i].lastScale.z;

        bool renderChanged =
            render.mesh != m_renderCache[i].instance.mesh ||
            render.material.get() != m_renderCache[i].instance.material;

        if (transformChanged || renderChanged)
        {
            RefreshRenderCacheEntry(i, &transform, &render);
        }

        ++i;
    }

    // Check for new entities
    auto renderArray = m_ecsComponentManager.GetComponentArray<ECS::RenderComponent>();
    auto& renderVec = renderArray->GetComponentArray();
    
    for (size_t i = 0; i < renderVec.size(); ++i) {
        ECS::Entity entity = renderArray->GetEntityAtIndex(i);
        
        if (m_entityToRenderCacheIndex.find(entity) != m_entityToRenderCacheIndex.end()) continue;
        
        ECS::RenderComponent& render = renderVec[i];
        if (!render.mesh || !render.material) continue;
        
        if (!m_ecsComponentManager.HasComponent<ECS::TransformComponent>(entity)) continue;
        ECS::TransformComponent& transform = m_ecsComponentManager.GetComponent<ECS::TransformComponent>(entity);
        
        CreateRenderCacheEntry(entity, &transform, &render);
    }
}

void Scene::RemoveRenderCacheEntry(size_t index)
{
    if (index >= m_renderCache.size()) return;

    ECS::Entity entity = m_renderCache[index].entity;
    m_entityToRenderCacheIndex.erase(entity);

    size_t lastIndex = m_renderCache.size() - 1;
    if (index != lastIndex)
    {
        std::swap(m_renderCache[index], m_renderCache[lastIndex]);
        m_entityToRenderCacheIndex[m_renderCache[index].entity] = index;
    }

    m_renderCache.pop_back();
}

void Scene::RefreshRenderCacheEntry(size_t index, const ECS::TransformComponent* transform, const ECS::RenderComponent* render)
{
    auto& entry = m_renderCache[index];
    entry.instance.mesh = render->mesh;
    entry.instance.material = render->material.get();
    entry.instance.position = transform->position;
    entry.instance.rotation = transform->rotation;
    entry.instance.scale = transform->scale;
    entry.instance.hasBounds = TryComputeWorldBounds(entry.entity, transform, entry.instance);

    entry.lastPosition = transform->position;
    entry.lastRotation = transform->rotation;
    entry.lastScale = transform->scale;
}

void Scene::CreateRenderCacheEntry(ECS::Entity entity, const ECS::TransformComponent* transform, const ECS::RenderComponent* render)
{
    RenderCacheEntry entry;
    entry.entity = entity;
    entry.instance.mesh = render->mesh;
    entry.instance.material = render->material.get();
    entry.instance.position = transform->position;
    entry.instance.rotation = transform->rotation;
    entry.instance.scale = transform->scale;
    entry.instance.hasBounds = TryComputeWorldBounds(entity, transform, entry.instance);
    entry.lastPosition = transform->position;
    entry.lastRotation = transform->rotation;
    entry.lastScale = transform->scale;

    m_entityToRenderCacheIndex[entity] = m_renderCache.size();
    m_renderCache.push_back(entry);
}

bool Scene::TryComputeWorldBounds(ECS::Entity entity, const ECS::TransformComponent* transform, Renderer::RenderInstance& instance)
{
    if (!transform) return false;

    auto computeFromLocal = [&](const AABB& localBounds)
    {
        instance.worldAABB.extents.x = std::abs(transform->scale.x) * localBounds.extents.x;
        instance.worldAABB.extents.y = std::abs(transform->scale.y) * localBounds.extents.y;
        instance.worldAABB.extents.z = std::abs(transform->scale.z) * localBounds.extents.z;

        instance.worldAABB.center.x = transform->position.x + transform->scale.x * localBounds.center.x;
        instance.worldAABB.center.y = transform->position.y + transform->scale.y * localBounds.center.y;
        instance.worldAABB.center.z = transform->position.z + transform->scale.z * localBounds.center.z;
    };

    if (m_ecsComponentManager.HasComponent<ECS::ColliderComponent>(entity))
    {
        auto& collider = m_ecsComponentManager.GetComponent<ECS::ColliderComponent>(entity);
        if (collider.enabled) {
            computeFromLocal(collider.localAABB);
            return true;
        }
    }

    if (m_ecsComponentManager.HasComponent<ECS::RenderComponent>(entity))
    {
        auto& render = m_ecsComponentManager.GetComponent<ECS::RenderComponent>(entity);
        if (render.mesh) {
            computeFromLocal(render.mesh->GetLocalBounds());
            return true;
        }
    }

    return false;
}

void Scene::GatherLights(std::vector<PointLight>& outLights)
{
    auto lightArray = m_ecsComponentManager.GetComponentArray<ECS::LightComponent>();
    auto& lightVec = lightArray->GetComponentArray();
    
    for (size_t i = 0; i < lightVec.size(); ++i) {
        ECS::Entity entity = lightArray->GetEntityAtIndex(i);
        ECS::LightComponent& light = lightVec[i];
        
        if (!light.enabled) continue;
        
        if (!m_ecsComponentManager.HasComponent<ECS::TransformComponent>(entity)) continue;
        ECS::TransformComponent& transform = m_ecsComponentManager.GetComponent<ECS::TransformComponent>(entity);
        
        PointLight pl;
        pl.position = DirectX::XMFLOAT4(transform.position.x, transform.position.y, transform.position.z, light.range);
        pl.color = light.color;
        // Default attenuation (Constant, Linear, Quadratic)
        pl.attenuation = DirectX::XMFLOAT4(1.0f, 0.09f, 0.032f, 0.0f); 
        outLights.push_back(pl);
    }
}

bool Scene::SetupCamera(Camera& outCamera, DirectX::XMMATRIX& outView, DirectX::XMMATRIX& outProj)
{
    if (!m_ecsCameraSystem->GetActiveCamera(outView, outProj)) {
        return false;
    }

    // Find player entity with camera to get position/rotation
    ECS::Entity cameraEntity = m_ecsCameraSystem->GetActiveCameraEntity();
    
    if (m_ecsComponentManager.HasComponent<ECS::TransformComponent>(cameraEntity)) {
        auto& transform = m_ecsComponentManager.GetComponent<ECS::TransformComponent>(cameraEntity);
        
        DirectX::XMFLOAT3 position = transform.position;
        
        if (m_ecsComponentManager.HasComponent<ECS::CameraComponent>(cameraEntity)) {
            auto& cameraComp = m_ecsComponentManager.GetComponent<ECS::CameraComponent>(cameraEntity);
            position.x += cameraComp.positionOffset.x;
            position.y += cameraComp.positionOffset.y;
            position.z += cameraComp.positionOffset.z;
        }

        float pitch = transform.rotation.x;
        if (m_ecsComponentManager.HasComponent<ECS::PlayerControllerComponent>(cameraEntity)) {
            auto& playerController = m_ecsComponentManager.GetComponent<ECS::PlayerControllerComponent>(cameraEntity);
            pitch = playerController.viewPitch;
        }
        
        outCamera.SetPosition(position.x, position.y, position.z);
        outCamera.SetRotation(pitch, transform.rotation.y, transform.rotation.z);
        return true;
    }
    return false;
}

void Scene::RenderUI(Renderer* renderer, UIRenderer* uiRenderer, bool showDebugCollision)
{
    uiRenderer->EnableUIState();

    if (m_crosshair) {
        m_crosshair->Draw(uiRenderer, m_font, 1280, 720);
    }

    // Render debug UI (can be toggled with F1)
    if (m_debugUI.IsEnabled()) {
        ECS::Entity activeCamera = m_ecsCameraSystem->GetActiveCameraEntity();
        m_debugUI.Render(
            uiRenderer, 
            m_font, 
            m_fps, 
            renderer->GetPostProcess()->IsBloomEnabled(),
            showDebugCollision,
            m_ecsComponentManager,
            activeCamera
        );
    }

    uiRenderer->DisableUIState();
}
