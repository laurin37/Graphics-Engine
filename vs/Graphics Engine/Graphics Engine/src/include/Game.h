#pragma once

#include "Window.h"
#include "Graphics.h"
#include "Input.h"
#include "Renderer.h"
#include "Camera.h"
#include "GameObject.h"
#include "Mesh.h" 
#include "SimpleFont.h"
#include "UIRenderer.h"
#include "Player.h"        // [NEW]
#include "PhysicsSystem.h" // [NEW]
#include "Bullet.h"        // Include Bullet header
#include "Gun.h"           // Include Gun header
#include "HealthObject.h"  // Include HealthObject header
#include <memory>
#include <vector>
#include <chrono>

class AssetManager; // Forward Declaration

class Game
{
public:
    Game();
    ~Game();

    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    void Run();

    // New method to spawn a bullet
    void SpawnBullet(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction, float speed, float damage);

private:
    void Update(float deltaTime);
    void Render();
    void LoadScene();

    // Helper method to add a bullet to internal vectors
    void AddBulletInternal(std::unique_ptr<Bullet> bullet);


    // System Objects
    Window m_window;
    Graphics m_graphics;
    Input m_input;

    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<AssetManager> m_assetManager;
    std::unique_ptr<UIRenderer> m_uiRenderer;

    // Scene Objects
    std::unique_ptr<Camera> m_camera;
    SimpleFont m_font;

    // Bullet and HealthObject assets (stored in Game to be easily accessible)
    std::shared_ptr<Mesh> m_bulletMesh;
    std::shared_ptr<Material> m_bulletMaterial;
    std::shared_ptr<Mesh> m_healthObjectMesh;
    std::shared_ptr<Material> m_healthObjectMaterial;

    // The actual objects in the scene (Owns the Player memory too)
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    std::vector<Bullet*> m_bullets; // New: To manage active bullets (non-owning pointers)

    // Lighting
    DirectionalLight m_dirLight;
    std::vector<PointLight> m_pointLights;

    // Physics & Player
    PhysicsSystem m_physics;
    Player* m_player = nullptr; // Non-owning pointer (memory owned by m_gameObjects)

    // Loop / Timing
    std::chrono::steady_clock::time_point m_lastTime;
    int m_fps = 0;
    int m_frameCount = 0;
    float m_timeAccum = 0.0f;
};