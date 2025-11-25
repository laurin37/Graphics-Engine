#pragma once

#include <DirectXMath.h>
#include <memory>
#include "../Physics/Collision.h"

// Forward declarations
class Mesh;
class Material;

namespace ECS {

// ========================================
// Transform Component
// Position, rotation, and scale in 3D space
// ========================================
struct TransformComponent {
    DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };  // Euler angles (radians)
    DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
};



// ========================================
// Physics Component
// Velocity, forces, and physics properties
// ========================================
struct PhysicsComponent {
    DirectX::XMFLOAT3 velocity = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 acceleration = { 0.0f, 0.0f, 0.0f };
    
    float mass = 1.0f;
    float drag = 0.0f;
    float gravityAcceleration = -15.0f;
    float maxFallSpeed = -15.0f;
    
    bool useGravity = true;
    bool checkCollisions = true;
    bool isGrounded = false;
};

// ========================================
// Render Component
// Mesh and material for rendering
// ========================================
struct RenderComponent {
    Mesh* mesh = nullptr;
    std::shared_ptr<Material> material;
};

// ========================================
// Collider Component
// Collision detection volume
// ========================================
struct ColliderComponent {
    AABB localAABB;     // Bounding box in local space
    bool enabled = true;
};

// ========================================
// Light Component
// Point light properties
// ========================================
struct LightComponent {
    DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;
    float range = 10.0f;
    bool enabled = true;
};

// ========================================
// Rotate Component
// Simple continuous rotation
// ========================================
struct RotateComponent {
    DirectX::XMFLOAT3 axis = { 0.0f, 1.0f, 0.0f };
    float speed = 1.0f; // Radians per second
};

// ========================================
// Orbit Component
// Orbit around a center point
// ========================================
struct OrbitComponent {
    DirectX::XMFLOAT3 center = { 0.0f, 0.0f, 0.0f };
    float radius = 5.0f;
    float speed = 1.0f;
    float angle = 0.0f; // Current angle
    DirectX::XMFLOAT3 axis = { 0.0f, 1.0f, 0.0f }; // Axis to orbit around
};

// ========================================
// PlayerController Component
// First-person player movement and controls
// ========================================
struct PlayerControllerComponent {
    float moveSpeed = 5.0f;
    float jumpForce = 7.0f;
    float mouseSensitivity = 0.002f;
    float cameraHeight = 0.7f; // Eye level offset from position
    bool canJump = true;
};

// ========================================
// Camera Component
// Camera properties and cached matrices
// ========================================
struct CameraComponent {
    float fov = 70.0f;                      // Field of view (degrees)
    float aspectRatio = 16.0f / 9.0f;       // Aspect ratio (width/height)
    float nearPlane = 0.1f;                 // Near clipping plane
    float farPlane = 1000.0f;               // Far clipping plane
    bool isActive = false;                  // Only one camera should be active
    DirectX::XMFLOAT3 positionOffset = { 0.0f, 0.0f, 0.0f }; // Offset from entity transform
    
    // Cached matrices (updated by CameraSystem)
    DirectX::XMFLOAT4X4 viewMatrix;
    DirectX::XMFLOAT4X4 projectionMatrix;
};

} // namespace ECS
