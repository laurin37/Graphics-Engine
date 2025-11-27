#include "Systems/PlayerMovementSystem.h"
#include <DirectXMath.h>
#include "Utils/Logger.h"

using namespace DirectX;

namespace ECS {

void PlayerMovementSystem::Init() {
    // Cache component arrays for performance
    m_controllerArray = m_componentManager.GetComponentArray<PlayerControllerComponent>();
    m_transformArray = m_componentManager.GetComponentArray<TransformComponent>();
    m_physicsArray = m_componentManager.GetComponentArray<PhysicsComponent>();
}

void PlayerMovementSystem::Update(float deltaTime) {
    // Use Query API to get entities with Controller, Transform, and Input
    auto entities = m_componentManager.QueryEntities<PlayerControllerComponent, TransformComponent, InputComponent>();
    
    for (Entity entity : entities) {
        auto& controller = m_componentManager.GetComponent<PlayerControllerComponent>(entity);
        auto& transform = m_componentManager.GetComponent<TransformComponent>(entity);
        auto& input = m_componentManager.GetComponent<InputComponent>(entity);
        
        // Handle mouse look and camera
        HandleMouseLook(entity, transform, controller, input, deltaTime);
        
        // Handle movement (WASD) and Jump
        if (m_componentManager.HasComponent<PhysicsComponent>(entity)) {
            auto& physics = m_componentManager.GetComponent<PhysicsComponent>(entity);
            HandleMovement(entity, transform, physics, controller, input, deltaTime);
        }
    }
}

void PlayerMovementSystem::HandleMovement(Entity entity, TransformComponent& transform, PhysicsComponent& physics,
                                         PlayerControllerComponent& controller, const InputComponent& input, float deltaTime) {
    // Get movement direction from input component
    XMFLOAT3 moveDir = { input.moveX, 0.0f, input.moveZ };
    
    // Normalize and apply speed
    XMVECTOR moveDirVec = XMLoadFloat3(&moveDir);
    float length = XMVectorGetX(XMVector3Length(moveDirVec));
    
    if (length > 0.0f) {
        // Input is already normalized by InputSystem, but safety check
        if (length > 1.0f) moveDirVec = XMVector3Normalize(moveDirVec);
        
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

    // Jump Logic
    if (input.jump && physics.isGrounded && controller.canJump) {
        physics.velocity.y = controller.jumpForce;
        physics.isGrounded = false; // Prevent multi-jump in same frame
    }
}

void PlayerMovementSystem::HandleMouseLook(Entity entity, TransformComponent& transform,
                                          PlayerControllerComponent& controller, 
                                          const InputComponent& input, float deltaTime) {
    // Get mouse delta from input component
    float mouseDeltaX = input.lookX;
    float mouseDeltaY = input.lookY;
    
    // Apply rotation (yaw = Y rotation, pitch = X rotation)
    transform.rotation.y += mouseDeltaX * controller.mouseSensitivity;  // Yaw
    controller.viewPitch += mouseDeltaY * controller.mouseSensitivity;  // Pitch
    
    // Clamp pitch to prevent flipping
    const float maxPitch = DirectX::XM_PI / 2.0f - 0.1f;
    if (controller.viewPitch > maxPitch) controller.viewPitch = maxPitch;
    if (controller.viewPitch < -maxPitch) controller.viewPitch = -maxPitch;

    // Keep the physical player mesh upright (only yaw affects it)
    transform.rotation.x = 0.0f;
    transform.rotation.z = 0.0f;
}

} // namespace ECS

