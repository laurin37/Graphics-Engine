#pragma once
#include "GameObject.h"
#include "Input.h"     
#include "Camera.h"    
#include "Gun.h"
#include "PhysicsBody.h"

// Forward declaration for Scene
class Scene;

class Player : public GameObject
{
public:
    Player(Mesh* mesh, std::shared_ptr<Material> material, Camera* camera);

    void Update(float deltaTime, Input& input, const std::vector<std::unique_ptr<GameObject>>& worldObjects);
    void Shoot(Scene* sceneInstance);

    void SetGun(Gun* gun) { m_gunPtr = gun; }

private:
    Camera* m_camera;
    Gun* m_gunPtr = nullptr;
    
    // Physics component (owned by Player)
    PhysicsBody m_physicsBody;
    
    // Player-specific constants
    const float MOVE_SPEED = 5.0f;
    const float JUMP_FORCE = 5.0f;
    
    // Helper methods
    void HandleInput(Input& input, float deltaTime);
    void UpdateCameraAndGun(float deltaTime);
};