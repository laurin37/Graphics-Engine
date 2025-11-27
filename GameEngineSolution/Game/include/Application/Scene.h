
#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <DirectXMath.h>

#include "Renderer/Camera.h"
#include "UI/SimpleFont.h"
#include "UI/Crosshair.h"
#include "UI/DebugUIRenderer.h"
#include "Renderer/Graphics.h"
#include "Renderer/Renderer.h"
#include "ECS/ComponentManager.h"
#include "ECS/SystemManager.h"
#include "Events/Event.h"
#include "Events/EventBus.h"
// Forward declarations for Systems
namespace ECS {
    class PhysicsSystem;
    class RenderSystem;
    class MovementSystem;
    class PlayerMovementSystem;
    class CameraSystem;
    class InputSystem;
}
class HealthSystem;
class WeaponSystem;
class ProjectileSystem;

// Forward declarations
class AssetManager;
class Renderer;
class UIRenderer;
class Input;
class Mesh;
class Material;
struct DirectionalLight;

// ==================================================================================
// Scene Class
// ----------------------------------------------------------------------------------
// Manages the game world, including entities, systems, and resources.
// It acts as the central hub for the ECS (Entity Component System) and handles:
// - Initialization of systems (Physics, Rendering, Gameplay)
// - Loading and management of game assets (Meshes, Textures)
// - The main Update loop (propagating time deltas to systems)
// - The Render loop (coordinating with the Renderer and UI)
// ==================================================================================
class Scene
{
public:
    Scene(AssetManager* assetManager, Graphics* graphics, Input* input, class EventBus* eventBus);
    ~Scene();

    void Load();
    void Update(float deltaTime);
    void Render(Renderer* renderer, UIRenderer* uiRenderer, bool showDebugCollision = false);
    
    // Scene loading from JSON
    void LoadSceneFromJSON(const std::wstring& jsonPath);

    // Debug UI
    void ToggleDebugUI() { m_debugUI.Toggle(); }
    bool IsDebugUIEnabled() const { return m_debugUI.IsEnabled(); }

private:
    // Render helpers
    bool SetupCamera(Camera& outCamera, DirectX::XMMATRIX& outView, DirectX::XMMATRIX& outProj);
    void RenderUI(Renderer* renderer, UIRenderer* uiRenderer, bool showDebugCollision);

    // Non-owning pointers
    AssetManager* m_assetManager;
    Graphics* m_graphics;
    Input* m_input;
    EventBus* m_eventBus;
    
    // Cached system pointers for direct access
    ECS::InputSystem* m_inputSystem = nullptr;
    ECS::PhysicsSystem* m_ecsPhysicsSystem = nullptr;
    ECS::RenderSystem* m_ecsRenderSystem = nullptr;
    ECS::MovementSystem* m_ecsMovementSystem = nullptr;
    ECS::PlayerMovementSystem* m_ecsPlayerMovementSystem = nullptr;
    ECS::CameraSystem* m_ecsCameraSystem = nullptr;
    HealthSystem* m_healthSystem = nullptr;
    WeaponSystem* m_weaponSystem = nullptr;
    ProjectileSystem* m_projectileSystem = nullptr;

    // Event subscriptions
    std::vector<EventBus::SubscriptionId> m_eventSubscriptions;

    DirectionalLight m_dirLight;
    DebugUIRenderer m_debugUI;
    
    // ECS Managers
    ECS::ComponentManager m_ecsComponentManager;
    ECS::SystemManager m_systemManager;

    // UI Elements
    std::unique_ptr<Crosshair> m_crosshair;
    SimpleFont m_font;

    // FPS Calculation
    float m_timeAccum = 0.0f;
    int m_frameCount = 0;
    int m_fps = 0;
};
