#include <algorithm>

#include "../../include/EntityComponentSystem/Camera.h"

using namespace DirectX;

Camera::Camera() : m_pos(0.0f, 0.0f, 0.0f), m_rot(0.0f, 0.0f, 0.0f) {}

void Camera::SetPosition(float x, float y, float z) { m_pos = { x, y, z }; }
void Camera::SetRotation(float x, float y, float z) { m_rot = { x, y, z }; }

XMVECTOR Camera::GetPosition() const { return XMLoadFloat3(&m_pos); }
XMFLOAT3 Camera::GetPositionFloat3() const { return m_pos; }
XMVECTOR Camera::GetRotation() const { return XMLoadFloat3(&m_rot); }

DirectX::XMVECTOR Camera::GetForward() const
{
    XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rot.x, m_rot.y, m_rot.z);
    return XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotMatrix);
}

DirectX::XMVECTOR Camera::GetRight() const
{
    XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rot.x, m_rot.y, m_rot.z);
    return XMVector3TransformCoord(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), rotMatrix);
}

DirectX::XMVECTOR Camera::GetUp() const
{
    XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rot.x, m_rot.y, m_rot.z);
    return XMVector3TransformCoord(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotMatrix);
}

void Camera::AdjustPosition(float x, float y, float z)
{
    XMVECTOR move = XMVectorSet(x, y, z, 0.0f);
    XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rot.x, m_rot.y, m_rot.z);
    XMVECTOR relativeMove = XMVector3TransformCoord(move, rotMatrix);
    XMStoreFloat3(&m_pos, XMLoadFloat3(&m_pos) + relativeMove);
}

void Camera::AdjustRotation(float x, float y, float z)
{
    m_rot.x += x;
    m_rot.y += y;
    m_rot.z += z;

    constexpr float pitchLimit = 1.55f; // Approx. 89 degrees
    m_rot.x = std::clamp(m_rot.x, -pitchLimit, pitchLimit);
}

XMMATRIX Camera::GetViewMatrix() const
{
    XMVECTOR pos = XMLoadFloat3(&m_pos);
    XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rot.x, m_rot.y, m_rot.z);

    XMVECTOR forward = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotMatrix);
    XMVECTOR up = XMVector3TransformCoord(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotMatrix);

    return XMMatrixLookAtLH(pos, pos + forward, up);
}
