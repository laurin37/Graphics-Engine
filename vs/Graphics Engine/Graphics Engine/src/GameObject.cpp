#include "include/GameObject.h"
#include "include/EnginePCH.h"
#include "include/Transform.h" // Include Transform.h

using namespace DirectX;

GameObject::GameObject(Mesh* mesh, std::shared_ptr<Material> material)
    : m_mesh(mesh), 
      m_material(material), 
      m_boundingBox({ {0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f} }),
      m_name(L"GameObject") // Default name
{}

void GameObject::SetMesh(Mesh* mesh)
{
    m_mesh = mesh;
    WCHAR buffer[256];
    swprintf_s(buffer, L"GameObject::SetMesh - Mesh address: 0x%p\n", mesh);
    OutputDebugString(buffer);
}

void GameObject::SetMaterial(std::shared_ptr<Material> material)
{
    m_material = material;
    WCHAR buffer[256];
    swprintf_s(buffer, L"GameObject::SetMaterial - Material address: 0x%p\n", material.get());
    OutputDebugString(buffer);
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
    AABB worldBox;
    // Note: This simple implementation does not account for rotation. 
    // For a more accurate AABB of a rotated object, one would need to transform the
    // 8 corners of the box and find the new min/max, which is more expensive.
    
    // 1. Scale the extents
    worldBox.extents.x = m_transform.GetScale().x * m_boundingBox.extents.x;
    worldBox.extents.y = m_transform.GetScale().y * m_boundingBox.extents.y;
    worldBox.extents.z = m_transform.GetScale().z * m_boundingBox.extents.z;

    // 2. Add scaled local center offset to world position
    worldBox.center.x = m_transform.GetPosition().x + (m_transform.GetScale().x * m_boundingBox.center.x);
    worldBox.center.y = m_transform.GetPosition().y + (m_transform.GetScale().y * m_boundingBox.center.y);
    worldBox.center.z = m_transform.GetPosition().z + (m_transform.GetScale().z * m_boundingBox.center.z);
    
    return worldBox;
}