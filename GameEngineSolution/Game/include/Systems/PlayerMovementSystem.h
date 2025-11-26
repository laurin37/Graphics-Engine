#pragma once

#include "ECS/ComponentManager.h"
#include "ECS/System.h"

// Forward declarations
class Input;

namespace ECS {

// ========================================
// PlayerMovementSystem
// Handles first-person player movement and camera control
// ========================================
class PlayerMovementSystem : public System {
public:
    PlayerMovementSystem(ComponentManager& cm, Input& input) 
        : System(cm), m_input(input) {}

    void Update(float deltaTime) override;

private:
    Input& m_input;
    void HandleMovement(Entity entity, TransformComponent& transform, PhysicsComponent& physics, 
                       PlayerControllerComponent& controller, Input& input, float deltaTime);
    void HandleJump(Entity entity, PhysicsComponent& physics, PlayerControllerComponent& controller, Input& input);
    void HandleMouseLook(Entity entity, TransformComponent& transform, PlayerControllerComponent& controller, 
                        Input& input, float deltaTime);
};

} // namespace ECS
