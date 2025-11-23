#pragma once
#include <DirectXMath.h>
#include <vector>
#include <memory>

// Forward declarations
class GameObject;

class PhysicsBody
{
public:
    // === Configuration ===
    bool useGravity = true;
    bool checkCollisions = true;
    bool isKinematic = false;  // If true, ignores forces (for manually controlled objects)
    float mass = 1.0f;
    float drag = 0.0f;         // Air resistance (0 = none, 1 = maximum)
    float maxFallSpeed = -15.0f;
    
    // Gravity constant
    float gravityAcceleration = -15.0f;
    
    // === State ===
    DirectX::XMFLOAT3 velocity;
    bool isGrounded = false;
    
    // === Public Methods ===
    PhysicsBody();
    
    // Set velocity directly
    void SetVelocity(const DirectX::XMFLOAT3& vel);
    void SetVelocity(float x, float y, float z);
    
    // Add force (force / mass = acceleration)
    void AddForce(const DirectX::XMFLOAT3& force);
    
    // Add impulse (instant velocity change)
    void AddImpulse(const DirectX::XMFLOAT3& impulse);
    
    // Main update - handles gravity, integration, and collision
    void Update(float dt, GameObject* owner, 
                const std::vector<std::unique_ptr<GameObject>>& worldObjects,
                const std::vector<GameObject*>& ignoreList);
    
private:
    // === Internal Physics Steps ===
    void ApplyGravity(float dt);
    void ApplyDrag(float dt);
    void ClampVelocity();
    
    void IntegrateVelocity(float dt, GameObject* owner);
    
    void ResolveCollisions(float dt, GameObject* owner,
                          const std::vector<std::unique_ptr<GameObject>>& worldObjects,
                          const std::vector<GameObject*>& ignoreList);
    
    void CheckGroundState(GameObject* owner,
                         const std::vector<std::unique_ptr<GameObject>>& worldObjects,
                         const std::vector<GameObject*>& ignoreList);
    
    // Accumulated forces (cleared each frame)
    DirectX::XMFLOAT3 m_accumulatedForces;
};
