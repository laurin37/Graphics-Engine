#include "../../../include/ECS/Systems/PlayerMovementSystem.h"
#include "../../../include/Input/Input.h"
#include "../../../include/EntityComponentSystem/Camera.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace ECS {

void PlayerMovementSystem::Update(ComponentManager& cm, Input& input, Camera& camera, float deltaTime) {
    // Get all player entities (entities with PlayerController + Transform)
    std::vector<Entity> players = cm.GetEntitiesWithPlayerControllerAndTransform();
    
    for (Entity player : players) {
        PlayerControllerComponent* controller = cm.GetPlayerController(player);
        TransformComponent* transform = cm.GetTransform(player);
        PhysicsComponent* physics = cm.GetPhysics(player);
        
        if (!controller || !transform) continue;
        
        // Handle mouse look and camera
        HandleMouseLook(player, *transform, *controller, camera, input, deltaTime);
        
        // Handle movement (WASD)
        if (physics) {
            HandleMovement(player, *transform, *physics, *controller, input, deltaTime);
            
            // Handle jump (Space)
            HandleJump(player, *physics, *controller, input);
        }
    }
}

void PlayerMovementSystem::HandleMovement(Entity entity, TransformComponent& transform, PhysicsComponent& physics,
                                         PlayerControllerComponent& controller, Input& input, float deltaTime) {
    // Get movement direction from input
    XMFLOAT3 moveDir = { 0.0f, 0.0f, 0.0f };
    
    if (input.IsKeyDown('W')) moveDir.z += 1.0f;
    if (input.IsKeyDown('S')) moveDir.z -= 1.0f;
    if (input.IsKeyDown('D')) moveDir.x += 1.0f;
    if (input.IsKeyDown('A')) moveDir.x -= 1.0f;
    
    // Normalize and apply speed
    XMVECTOR moveDirVec = XMLoadFloat3(&moveDir);
    float length = XMVectorGetX(XMVector3Length(moveDirVec));
    
    if (length > 0.0f) {
        moveDirVec = XMVector3Normalize(moveDirVec);
        
        // Rotate movement direction based on player's Y rotation (yaw)
        float yaw = transform.rotation.y;
        XMMATRIX rotMatrix = XMMatrixRotationY(yaw);
        moveDirVec = XMVector3Transform(moveDirVec, rotMatrix);
        
        // Apply horizontal movement (preserve vertical velocity)
        XMFLOAT3 finalMove;
        XMStoreFloat3(&finalMove, moveDirVec);
        
        physics.velocity.x = finalMove.x * controller.moveSpeed;
        physics.velocity.z = finalMove.z * controller.moveSpeed;
    } else {
        // No input, stop horizontal movement
        physics.velocity.x = 0.0f;
        physics.velocity.z = 0.0f;
    }
}

void PlayerMovementSystem::HandleJump(Entity entity, PhysicsComponent& physics, 
                                     PlayerControllerComponent& controller, Input& input) {
    // Jump on Space key (only if grounded)
    if (input.IsKeyDown(VK_SPACE) && physics.isGrounded && controller.canJump) {
        physics.velocity.y = controller.jumpForce;
    }
}

void PlayerMovementSystem::HandleMouseLook(Entity entity, TransformComponent& transform,
                                          PlayerControllerComponent& controller, Camera& camera, 
                                          Input& input, float deltaTime) {
    // Get mouse delta
    float mouseDeltaX = input.GetMouseDeltaX();
    float mouseDeltaY = input.GetMouseDeltaY();
    
    // Apply rotation (yaw = Y rotation, pitch = X rotation)
    transform.rotation.y += mouseDeltaX * controller.mouseSensitivity;  // Yaw
    transform.rotation.x -= mouseDeltaY * controller.mouseSensitivity;  // Pitch (inverted)
    
    // Clamp pitch to prevent flipping
    const float maxPitch = DirectX::XM_PI / 2.0f - 0.1f;
    if (transform.rotation.x > maxPitch) transform.rotation.x = maxPitch;
    if (transform.rotation.x < -maxPitch) transform.rotation.x = -maxPitch;
    
    // Update camera position and rotation
    XMFLOAT3 cameraPos = transform.position;
    cameraPos.y += controller.cameraHeight; // Eye level
    
    camera.SetPosition(cameraPos.x, cameraPos.y, cameraPos.z);
    camera.SetRotation(transform.rotation.x, transform.rotation.y, transform.rotation.z);
}

} // namespace ECS
