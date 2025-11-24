#pragma once
#include "Collision.h"
#include <DirectXMath.h>

// Forward declarations
class GameObject;
class Mesh;

enum class ColliderType
{
    AABB,      // Axis-Aligned Bounding Box (fast, loose fit when rotated)
    Sphere,    // Bounding sphere (fast, very loose fit)
    // OBB     // Oriented Bounding Box (future feature)
};

class Collider
{
public:
    // === Configuration ===
    ColliderType type = ColliderType::AABB;
    bool enabled = true;
    
    // === Construction ===
    Collider();
    
    // Auto-generate collider from mesh geometry
    void GenerateFromMesh(const Mesh* mesh, ColliderType colliderType = ColliderType::AABB);
    
    // Manual setup (for custom shapes)
    void SetLocalAABB(const AABB& aabb);
    
    // Get world-space bounding volume
    AABB GetWorldAABB(const GameObject* owner) const;
    
    // Get local-space AABB
    const AABB& GetLocalAABB() const { return m_localAABB; }
    
private:
    AABB m_localAABB; // In object's local space (before transform)
};
