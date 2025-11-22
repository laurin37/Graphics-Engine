#include "include/GameObject.h"
#include "include/EnginePCH.h"

using namespace DirectX;

GameObject::GameObject(Mesh* mesh, std::shared_ptr<Material> material)
    : m_pMesh(mesh), 
      m_material(material), 
      m_pos({0,0,0}), 
      m_rot({0,0,0}), 
      m_scale({1,1,1}),
      m_boundingBox({ {0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f} })
{}

void GameObject::SetPosition(float x, float y, float z) { m_pos = { x, y, z }; }
void GameObject::SetRotation(float x, float y, float z) { m_rot = { x, y, z }; }
void GameObject::SetScale(float x, float y, float z) { m_scale = { x, y, z }; }

DirectX::XMFLOAT3 GameObject::GetPosition() const
{
    return m_pos;
}

XMMATRIX GameObject::GetWorldMatrix() const
{
    return XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) *
           XMMatrixRotationRollPitchYaw(m_rot.x, m_rot.y, m_rot.z) *
           XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
}

void GameObject::Draw(ID3D11DeviceContext* context, ID3D11Buffer* psMaterialConstantBuffer) const
{
    if (m_material)
    {
        m_material->Bind(context, psMaterialConstantBuffer);
    }

    if (m_pMesh)
    {
        m_pMesh->Draw(context);
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
    worldBox.extents.x = m_scale.x * m_boundingBox.extents.x;
    worldBox.extents.y = m_scale.y * m_boundingBox.extents.y;
    worldBox.extents.z = m_scale.z * m_boundingBox.extents.z;

    // 2. Add scaled local center offset to world position
    worldBox.center.x = m_pos.x + (m_scale.x * m_boundingBox.center.x);
    worldBox.center.y = m_pos.y + (m_scale.y * m_boundingBox.center.y);
    worldBox.center.z = m_pos.z + (m_scale.z * m_boundingBox.center.z);
    
    return worldBox;
}