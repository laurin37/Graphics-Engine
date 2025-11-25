
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
#include "../ECS/Systems/ECSPhysicsSystem.h"
#include "../ECS/Systems/ECSRenderSystem.h"
#include "../ECS/Systems/ECSMovementSystem.h"
#include "../ECS/Systems/PlayerMovementSystem.h"
#include "../ECS/Systems/CameraSystem.h"

// Forward declarations
class AssetManager;
class Renderer;
class UIRenderer;
class Input;
class Mesh;
class Material;
struct DirectionalLight;

class Scene
{
public:
    Scene(AssetManager* assetManager, Graphics* graphics);
    ~Scene();

    void Load();
    void Update(float deltaTime, Input& input);
    void Render(Renderer* renderer, UIRenderer* uiRenderer, bool showDebugCollision = false);
    
    // Scene loading from JSON
    void LoadSceneFromJSON(const std::wstring& jsonPath);

    // Debug UI control
    void ToggleDebugUI() { m_debugUI.Toggle(); }
    bool IsDebugUIEnabled() const { return m_debugUI.IsEnabled(); }
    void SetDebugUIEnabled(bool enabled) { m_debugUI.SetEnabled(enabled); }

private:
    // Asset lookup for JSON scene loading
    std::unordered_map<std::string, Mesh*> BuildMeshLookup();
    std::unordered_map<std::string, std::shared_ptr<Material>> BuildMaterialLookup();
    void RebuildRenderCache();
    void UpdateRenderCache();
    void RemoveRenderCacheEntry(size_t index);
    void RefreshRenderCacheEntry(size_t index, const ECS::TransformComponent* transform, const ECS::RenderComponent* render);
    void CreateRenderCacheEntry(ECS::Entity entity, const ECS::TransformComponent* transform, const ECS::RenderComponent* render);
    bool TryComputeWorldBounds(ECS::Entity entity, const ECS::TransformComponent* transform, Renderer::RenderInstance& instance);

    // Non-owning pointers
    AssetManager* m_assetManager;
    Graphics* m_graphics;

    // Core systems
    SimpleFont m_font;
    std::unique_ptr<Crosshair> m_crosshair;
    DebugUIRenderer m_debugUI;

    // Meshes
    std::shared_ptr<Mesh> m_meshCube;
    std::shared_ptr<Mesh> m_meshCylinder;
    std::shared_ptr<Mesh> m_meshCone;
    std::shared_ptr<Mesh> m_meshSphere;
    std::shared_ptr<Mesh> m_meshTorus;
    std::shared_ptr<Mesh> m_meshRoom;

    // Materials
    std::shared_ptr<Material> m_matFloor;
    std::shared_ptr<Material> m_matPillar;
    std::shared_ptr<Material> m_matRoof;
    std::shared_ptr<Material> m_matGold;
    std::shared_ptr<Material> m_matGlowing;
    std::shared_ptr<Material> m_matOrbRed;
    std::shared_ptr<Material> m_matOrbGreen;
    std::shared_ptr<Material> m_matOrbBlue;
    std::shared_ptr<Material> m_matOrbOrange;

    // Lighting
    DirectionalLight m_dirLight;

    // FPS tracking
    int m_fps = 0;
    int m_frameCount = 0;
    float m_timeAccum = 0.0f;
    
    // ECS
    ECS::ComponentManager m_ecsComponentManager;
    ECS::PhysicsSystem m_ecsPhysicsSystem;
    ECS::RenderSystem m_ecsRenderSystem;
    ECS::MovementSystem m_ecsMovementSystem;
    ECS::PlayerMovementSystem m_ecsPlayerMovementSystem;
    ECS::CameraSystem m_ecsCameraSystem;

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
};
