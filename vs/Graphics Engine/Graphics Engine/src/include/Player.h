#pragma once
#include "GameObject.h"
#include "Input.h"     // Assuming you have an Input header
#include "Camera.h"    // Assuming you have a Camera header
#include "Gun.h"       // Include Gun header

// Forward declaration for Game to avoid circular dependency
class Game;

class Player : public GameObject
{
public:
    Player(Mesh* mesh, std::shared_ptr<Material> material, Camera* camera);

    void Update(float deltaTime, Input& input, const std::vector<std::unique_ptr<GameObject>>& worldObjects);
    void Shoot(Game* gameInstance); // Method to handle shooting

    void SetGun(Gun* gun) { m_gunPtr = gun; } // Made public

private:
    Camera* m_camera;
    DirectX::XMFLOAT3 m_velocity; // Re-added
    bool m_onGround;              // Re-added
    Gun* m_gunPtr = nullptr; // Player's gun (non-owning pointer)
    const float MOVE_SPEED = 5.0f;
    const float JUMP_FORCE = 5.0f;
    const float GRAVITY = -15.0f; // Higher gravity feels better in games
};