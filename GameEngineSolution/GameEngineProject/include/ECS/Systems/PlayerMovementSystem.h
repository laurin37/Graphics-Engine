#pragma once

#include "../ComponentManager.h"

// Forward declarations
class Input;
class Camera;

namespace ECS {

// ========================================
// PlayerMovementSystem
// Handles first-person player movement and camera control
// ========================================
class PlayerMovementSystem {
public:
    void Update(ComponentManager& cm, Input& input, Camera& camera, float deltaTime);

private:
    void HandleMovement(Entity entity, TransformComponent& transform, PhysicsComponent& physics, 
                       PlayerControllerComponent& controller, Input& input, float deltaTime);
    void HandleJump(Entity entity, PhysicsComponent& physics, PlayerControllerComponent& controller, Input& input);
    void HandleMouseLook(Entity entity, TransformComponent& transform, PlayerControllerComponent& controller, 
                        Camera& camera, Input& input, float deltaTime);
};

} // namespace ECS
