#include "../../include/Physics/Collider.h"
#include "../../include/EntityComponentSystem/GameObject.h"
#include "../../include/Renderer/Mesh.h"
#include "../../include/Renderer/MeshUtils.h"

using namespace DirectX;

Collider::Collider()
    : m_localAABB{ {0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f} }
{
}

void Collider::GenerateFromMesh(const Mesh* mesh, ColliderType colliderType)
{
    if (!mesh) {
        // Use default box if no mesh
        m_localAABB = AABB{ {0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f} };
        return;
    }
    
    type = colliderType;
    
    switch (type) {
        case ColliderType::AABB:
            m_localAABB = MeshUtils::CalculateAABB(mesh);
            break;
            
        case ColliderType::Sphere: {
            // Generate sphere from AABB max extent for tighter fit
            // (CalculateBoundingSphere uses diagonal which is too large for sphere meshes)
            AABB aabb = MeshUtils::CalculateAABB(mesh);
            float radius = (std::max)({aabb.extents.x, aabb.extents.y, aabb.extents.z});
            
            m_localAABB.center = aabb.center;
            m_localAABB.extents = { radius, radius, radius };
            break;
        }
    }
}

void Collider::SetLocalAABB(const AABB& aabb)
{
    m_localAABB = aabb;
    type = ColliderType::AABB;
}

AABB Collider::GetWorldAABB(const GameObject* owner) const
{
    if (!owner || !enabled) {
        return m_localAABB;
    }
    
    AABB worldBox;
    
    // 1. Scale the extents
    XMFLOAT3 scale = owner->GetScale(); // Fixed: was GetPosition()
    worldBox.extents.x = scale.x * m_localAABB.extents.x;
    worldBox.extents.y = scale.y * m_localAABB.extents.y;
    worldBox.extents.z = scale.z * m_localAABB.extents.z;
    
    // 2. Position = owner position + scaled local center offset
    XMFLOAT3 scaledCenter;
    scaledCenter.x = m_localAABB.center.x * scale.x;
    scaledCenter.y = m_localAABB.center.y * scale.y;
    scaledCenter.z = m_localAABB.center.z * scale.z;
    
    XMFLOAT3 ownerPos = owner->GetPosition();
    worldBox.center.x = ownerPos.x + scaledCenter.x;
    worldBox.center.y = ownerPos.y + scaledCenter.y;
    worldBox.center.z = ownerPos.z + scaledCenter.z;
    
    return worldBox;
}
