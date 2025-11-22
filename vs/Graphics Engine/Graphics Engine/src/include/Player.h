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
    void Shoot(Scene* sceneInstance); // Method to handle shooting

    void SetGun(Gun* gun) { m_gunPtr = gun; } // Made public

private:
    Camera* m_camera;
    DirectX::XMFLOAT3 m_velocity; 
    bool m_onGround;              
    Gun* m_gunPtr = nullptr; // Player's gun (non-owning pointer)
    const float MOVE_SPEED = 5.0f;
    const float JUMP_FORCE = 5.0f;
    const float GRAVITY = -15.0f; 
};