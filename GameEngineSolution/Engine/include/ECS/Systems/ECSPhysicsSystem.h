#pragma once

#include "../ComponentManager.h"
#include "../System.h"

namespace ECS {

// ========================================
// PhysicsSystem
// Handles physics simulation for entities
// with PhysicsComponent + TransformComponent
// ========================================
class PhysicsSystem : public System {
public:
    explicit PhysicsSystem(ComponentManager& cm) : System(cm) {}
    
    // Update all physics entities
    void Update(float deltaTime) override;
    
private:
    // Physics sub-steps
    void ApplyGravity(PhysicsComponent& physics, float dt);
    void ApplyDrag(PhysicsComponent& physics, float dt);
    void ClampVelocity(PhysicsComponent& physics);
    void IntegrateVelocity(TransformComponent& transform, PhysicsComponent& physics, float dt);
    
    // Collision (simplified - no GameObject dependency)
    void CheckGroundCollision(Entity entity, TransformComponent& transform, PhysicsComponent& physics);
    
    // Physics constants
    static constexpr float MIN_DELTA_TIME = 0.0001f;
    static constexpr float MAX_DELTA_TIME = 0.1f;
};

} // namespace ECS
