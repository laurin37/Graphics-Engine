#pragma once
#include "GameObject.h"
#include "Input.h"     
#include "Camera.h"    
#include "Gun.h"       

// Forward declaration for Scene to avoid circular dependency
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
    DirectX::XMFLOAT3 m_velocity; 
    bool m_onGround;              
    Gun* m_gunPtr = nullptr;
    const float MOVE_SPEED = 5.0f;
    const float JUMP_FORCE = 5.0f;
    const float GRAVITY = -15.0f;
    
    // Helper methods for fixed timestep
    void UpdatePhysicsStep(float dt, Input& input, const std::vector<std::unique_ptr<GameObject>>& worldObjects);
    void UpdateCameraAndGun(Input& input, float deltaTime);
};