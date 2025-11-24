
#pragma once

#include <vector>
#include <memory>

#include "../EntityComponentSystem/GameObject.h"
#include "../EntityComponentSystem/Camera.h"
#include "../Physics/PhysicsSystem.h"
#include "../EntityComponentSystem/Bullet.h"
#include "../EntityComponentSystem/Player.h"
#include "../UI/SimpleFont.h"
#include "../UI/Crosshair.h"
#include "../Renderer/Graphics.h"
#include "../ECS/ComponentManager.h"
#include "../ECS/Systems/ECSPhysicsSystem.h"
#include "../ECS/Systems/ECSRenderSystem.h"
#include "../ECS/Systems/ECSMovementSystem.h"
#include "../ECS/ECSExample.h"

class AssetManager;
class Renderer;
class UIRenderer;
class Input;

class Scene
{
public:
    Scene(AssetManager* assetManager, Graphics* graphics);
    ~Scene();

    void Load();
    void Update(float deltaTime, Input& input);
    void Render(Renderer* renderer, UIRenderer* uiRenderer, bool showDebugCollision = false);

    void SpawnBullet(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction, float speed, float damage);
    
    // ECS Control
    void ToggleECS() { m_useECS = !m_useECS; }
    bool IsUsingECS() const { return m_useECS; }

private:
    void AddBulletInternal(std::unique_ptr<Bullet> bullet);
    
    // Scene loading helpers
    void SetupCamera();
    void SetupLighting();
    void LoadAssets();
    void CreateMaterials();
    void SpawnSceneObjects();
    void InitializeUI();

    AssetManager* m_assetManager; // Non-owning
    Graphics* m_graphics;         // Non-owning (for creating debug font texture if needed, though usually AssetManager handles assets)

    // Scene Objects
    std::unique_ptr<Camera> m_camera;
    SimpleFont m_font;

    // Shared assets loaded in LoadAssets()
    std::shared_ptr<Mesh> m_meshCube;
    std::shared_ptr<Mesh> m_meshCylinder;
    std::shared_ptr<Mesh> m_meshCone;
    std::shared_ptr<Mesh> m_meshSphere;
    std::shared_ptr<Mesh> m_meshTorus;
    std::shared_ptr<Mesh> m_meshRoom;

    // Shared materials created in CreateMaterials()
    std::shared_ptr<Material> m_matFloor;
    std::shared_ptr<Material> m_matPillar;
    std::shared_ptr<Material> m_matRoof;
    std::shared_ptr<Material> m_matGold;
    std::shared_ptr<Material> m_matGlowing;
    
    // Individual orb materials (emissive)
    std::shared_ptr<Material> m_matOrbRed;
    std::shared_ptr<Material> m_matOrbGreen;
    std::shared_ptr<Material> m_matOrbBlue;
    std::shared_ptr<Material> m_matOrbOrange;

    // Bullet and HealthObject assets
    std::shared_ptr<Mesh> m_bulletMesh;
    std::shared_ptr<Material> m_bulletMaterial;
    std::shared_ptr<Mesh> m_healthObjectMesh;
    std::shared_ptr<Material> m_healthObjectMaterial;

    // Objects
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    std::vector<Bullet*> m_bullets; 

    // Lighting
    DirectionalLight m_dirLight;
    std::vector<PointLight> m_pointLights;

    // Systems
    PhysicsSystem m_physics;
    Player* m_player = nullptr; 
    std::unique_ptr<Crosshair> m_crosshair;

    // FPS / Stats (optional, can stay in Game or move here)
    int m_fps = 0;
    int m_frameCount = 0;
    float m_timeAccum = 0.0f;
    
    // ECS
    ECS::ComponentManager m_ecsComponentManager;
    ECS::PhysicsSystem m_ecsPhysicsSystem;
    ECS::RenderSystem m_ecsRenderSystem;
    ECS::MovementSystem m_ecsMovementSystem;
    
    bool m_useECS = false;  // Toggle between GameObject and ECS systems
};
