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
#include "ECS/Systems/InputSystem.h"
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
#include "Events/ECSEvents.h"

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
    // Note: Order of adding systems doesn't matter - SystemPhase controls execution order
    
    // Set EventBus for SystemManager so it's passed to all new systems automatically
    m_systemManager.SetEventBus(m_eventBus);
    m_ecsComponentManager.SetEventBus(m_eventBus);
    
    // 1. Core Systems
    m_inputSystem = m_systemManager.AddSystem<ECS::InputSystem>(m_ecsComponentManager, *m_input);
    m_ecsPhysicsSystem = m_systemManager.AddSystem<ECS::PhysicsSystem>(m_ecsComponentManager);
    m_ecsMovementSystem = m_systemManager.AddSystem<ECS::MovementSystem>(m_ecsComponentManager);
    m_ecsCameraSystem = m_systemManager.AddSystem<ECS::CameraSystem>(m_ecsComponentManager);
    
    // 2. Gameplay Systems (depend on Input)
    if (m_input) {
        m_ecsPlayerMovementSystem = m_systemManager.AddSystem<ECS::PlayerMovementSystem>(m_ecsComponentManager);
        m_weaponSystem = m_systemManager.AddSystem<WeaponSystem>(m_ecsComponentManager);
        m_weaponSystem->SetPhysicsSystem(m_ecsPhysicsSystem);
    }
    m_projectileSystem = m_systemManager.AddSystem<ProjectileSystem>(m_ecsComponentManager);
    m_healthSystem = m_systemManager.AddSystem<HealthSystem>(m_ecsComponentManager);
    
    // 3. Rendering System (needs to be updated manually or last)
    m_ecsRenderSystem = m_systemManager.AddSystem<ECS::RenderSystem>(m_ecsComponentManager);
    
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
    // ========================================
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

    // Force rebuild of render cache to ensure all loaded entities are visible
    if (m_ecsRenderSystem) {
        m_ecsRenderSystem->RebuildRenderCache();
    }
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
    
    // Get ECS camera and create temporary Camera adapter for renderer
    DirectX::XMMATRIX ecsView, ecsProj;
    Camera tempCamera;  // Temporary adapter
    
    if (SetupCamera(tempCamera, ecsView, ecsProj)) {
        // Render scene via ECS System
        m_ecsRenderSystem->Render(renderer, tempCamera, m_dirLight);
        
        // Render debug collision boxes
        if (showDebugCollision) {
            m_ecsRenderSystem->RenderDebug(renderer, tempCamera);
        }
    }
    
    // Render UI
    RenderUI(renderer, uiRenderer, showDebugCollision);
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
