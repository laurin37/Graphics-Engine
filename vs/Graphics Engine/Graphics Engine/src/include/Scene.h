#pragma once

#include "GameObject.h"
#include "Camera.h"
#include "PhysicsSystem.h"
#include "Bullet.h"
#include "Player.h"
#include "SimpleFont.h"
#include "Crosshair.h"
#include "Graphics.h" // For lights definitions
#include <vector>
#include <memory>

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
    void Render(Renderer* renderer, UIRenderer* uiRenderer);

    void SpawnBullet(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction, float speed, float damage);

private:
    void AddBulletInternal(std::unique_ptr<Bullet> bullet);

    AssetManager* m_assetManager; // Non-owning
    Graphics* m_graphics;         // Non-owning (for creating debug font texture if needed, though usually AssetManager handles assets)

    // Scene Objects
    std::unique_ptr<Camera> m_camera;
    SimpleFont m_font;

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
};
