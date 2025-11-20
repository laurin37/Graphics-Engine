#include "GameObject.h"

using namespace DirectX;

GameObject::GameObject(Mesh* mesh, std::shared_ptr<Material> material)
    : m_pMesh(mesh), m_material(material), m_pos({0,0,0}), m_rot({0,0,0}), m_scale({1,1,1})
{}

void GameObject::SetPosition(float x, float y, float z) { m_pos = { x, y, z }; }
void GameObject::SetRotation(float x, float y, float z) { m_rot = { x, y, z }; }
void GameObject::SetScale(float x, float y, float z) { m_scale = { x, y, z }; }

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
