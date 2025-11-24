#include "../../include/EntityComponentSystem/GameObject.h"
#include "../../include/Utils/EnginePCH.h"
#include "../../include/Utils/Transform.h"
#include "../../include/Physics/Collider.h"

using namespace DirectX;

GameObject::GameObject(Mesh* mesh, std::shared_ptr<Material> material)
    : m_mesh(mesh), 
      m_material(material), 
      m_boundingBox({ {0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f} }),
      m_name(L"GameObject")
{}

void GameObject::SetMesh(Mesh* mesh)
{
    m_mesh = mesh;
    //LOG_DEBUG(std::format("GameObject::SetMesh - Mesh address: {:p}", static_cast<void*>(mesh)));
}

void GameObject::SetMaterial(std::shared_ptr<Material> material)
{
    m_material = material;
    //LOG_DEBUG(std::format("GameObject::SetMaterial - Material address: {:p}", static_cast<void*>(material.get())));
}

void GameObject::Draw(ID3D11DeviceContext* context, ID3D11Buffer* psMaterialConstantBuffer) const
{
    if (m_material)
    {
        m_material->Bind(context, psMaterialConstantBuffer);
    }

    if (m_mesh)
    {
        m_mesh->Draw(context);
    }
}

void GameObject::SetBoundingBox(const AABB& box)
{
    m_boundingBox = box;
}

AABB GameObject::GetWorldBoundingBox() const
{
    // If collider component exists, use it
    if (m_collider) {
        return m_collider->GetWorldAABB(this);
    }
    
    // Otherwise use legacy bounding box
    AABB worldBox;
    
    worldBox.extents.x = m_transform.GetScale().x * m_boundingBox.extents.x;
    worldBox.extents.y = m_transform.GetScale().y * m_boundingBox.extents.y;
    worldBox.extents.z = m_transform.GetScale().z * m_boundingBox.extents.z;
    
    worldBox.center.x = m_transform.GetPosition().x + (m_transform.GetScale().x * m_boundingBox.center.x);
    worldBox.center.y = m_transform.GetPosition().y + (m_transform.GetScale().y * m_boundingBox.center.y);
    worldBox.center.z = m_transform.GetPosition().z + (m_transform.GetScale().z * m_boundingBox.center.z);
    
    return worldBox;
}

void GameObject::GenerateCollider(ColliderType type)
{
    // Use Collider temporarily to calculate bounds from mesh
    Collider tempCollider;
    if (m_mesh) {
        tempCollider.GenerateFromMesh(m_mesh, type);
        
        // Store in legacy bounding box for now
        m_boundingBox = tempCollider.GetLocalAABB();
    }
}