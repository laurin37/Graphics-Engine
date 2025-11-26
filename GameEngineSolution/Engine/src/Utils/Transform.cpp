#include "../../include/Utils/Transform.h"

using namespace DirectX;

Transform::Transform()
    : m_pos({0,0,0}), 
      m_rot({0,0,0}), 
      m_scale({1,1,1})
{}

void Transform::SetPosition(float x, float y, float z) { m_pos = { x, y, z }; }
void Transform::SetRotation(float x, float y, float z) { m_rot = { x, y, z }; }
void Transform::SetScale(float x, float y, float z) { m_scale = { x, y, z }; }

XMMATRIX Transform::GetWorldMatrix() const
{
    return XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) *
           XMMatrixRotationRollPitchYaw(m_rot.x, m_rot.y, m_rot.z) *
           XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
}