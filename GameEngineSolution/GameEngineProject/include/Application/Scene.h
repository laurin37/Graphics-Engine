
#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

#include "../EntityComponentSystem/GameObject.h"
#include "../EntityComponentSystem/Camera.h"
#include "../UI/SimpleFont.h"
#include "../UI/Crosshair.h"
#include "../Renderer/Graphics.h"
#include "../ECS/ComponentManager.h"
#include "../ECS/Systems/ECSPhysicsSystem.h"
#include "../ECS/Systems/ECSRenderSystem.h"
#include "../ECS/Systems/ECSMovementSystem.h"
#include "../ECS/Systems/PlayerMovementSystem.h"

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

private:
    // Asset lookup for JSON scene loading
    std::unordered_map<std::string, Mesh*> BuildMeshLookup();
    std::unordered_map<std::string, std::shared_ptr<Material>> BuildMaterialLookup();

    // Non-owning pointers
    AssetManager* m_assetManager;
    Graphics* m_graphics;

    // Core systems
    std::unique_ptr<Camera> m_camera;
    SimpleFont m_font;
    std::unique_ptr<Crosshair> m_crosshair;

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
};
