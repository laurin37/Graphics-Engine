
#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <DirectXMath.h>

#include "../EntityComponentSystem/Camera.h"
#include "../UI/SimpleFont.h"
#include "../UI/Crosshair.h"
#include "../UI/DebugUIRenderer.h"
#include "../Renderer/Graphics.h"
#include "../Renderer/Renderer.h"
#include "../ECS/ComponentManager.h"
#include "../ECS/SystemManager.h"

// Forward declarations for Systems
namespace ECS {
    class PhysicsSystem;
    class RenderSystem;
    class MovementSystem;
    class PlayerMovementSystem;
    class CameraSystem;
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
    Scene(AssetManager* assetManager, Graphics* graphics, Input* input);
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
    void RebuildRenderCache();
    void UpdateRenderCache();
    void CreateRenderCacheEntry(ECS::Entity entity, const ECS::TransformComponent* transform, const ECS::RenderComponent* render);
    void RemoveRenderCacheEntry(size_t index);
    void RefreshRenderCacheEntry(size_t index, const ECS::TransformComponent* transform, const ECS::RenderComponent* render);
    bool TryComputeWorldBounds(ECS::Entity entity, const ECS::TransformComponent* transform, Renderer::RenderInstance& instance);

    // Render helpers
    void GatherLights(std::vector<PointLight>& outLights);
    bool SetupCamera(Camera& outCamera, DirectX::XMMATRIX& outView, DirectX::XMMATRIX& outProj);
    void RenderUI(Renderer* renderer, UIRenderer* uiRenderer, bool showDebugCollision);

public:
    // Non-owning pointers
    AssetManager* m_assetManager;
    Graphics* m_graphics;
    Input* m_input;
    // Cached system pointers for direct access
    ECS::PhysicsSystem* m_ecsPhysicsSystem = nullptr;
    ECS::RenderSystem* m_ecsRenderSystem = nullptr;
    ECS::MovementSystem* m_ecsMovementSystem = nullptr;
    ECS::PlayerMovementSystem* m_ecsPlayerMovementSystem = nullptr;
    ECS::CameraSystem* m_ecsCameraSystem = nullptr;
    HealthSystem* m_healthSystem = nullptr;
    WeaponSystem* m_weaponSystem = nullptr;
    ProjectileSystem* m_projectileSystem = nullptr;

    struct RenderCacheEntry
    {
        ECS::Entity entity = ECS::NULL_ENTITY;
        Renderer::RenderInstance instance{};
        DirectX::XMFLOAT3 lastPosition{ 0.0f,0.0f,0.0f };
        DirectX::XMFLOAT3 lastRotation{ 0.0f,0.0f,0.0f };
        DirectX::XMFLOAT3 lastScale{ 1.0f,1.0f,1.0f };
    };

    std::vector<RenderCacheEntry> m_renderCache;
    std::unordered_map<ECS::Entity, size_t> m_entityToRenderCacheIndex;

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
