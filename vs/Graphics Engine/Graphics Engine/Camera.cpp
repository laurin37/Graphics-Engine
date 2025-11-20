#include "Camera.h"

using namespace DirectX;

Camera::Camera() : m_pos(0.0f, 0.0f, 0.0f), m_rot(0.0f, 0.0f, 0.0f) {}

void Camera::SetPosition(float x, float y, float z) { m_pos = { x, y, z }; }
void Camera::SetRotation(float x, float y, float z) { m_rot = { x, y, z }; }

XMVECTOR Camera::GetPosition() const { return XMLoadFloat3(&m_pos); }
XMFLOAT3 Camera::GetPositionFloat3() const { return m_pos; }
XMVECTOR Camera::GetRotation() const { return XMLoadFloat3(&m_rot); }

void Camera::Move(float x, float y, float z) { m_pos.x += x; m_pos.y += y; m_pos.z += z; }
void Camera::Rotate(float x, float y, float z) { m_rot.x += x; m_rot.y += y; m_rot.z += z; }

XMMATRIX Camera::GetViewMatrix() const
{
    XMVECTOR pos = XMLoadFloat3(&m_pos);
    XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rot.x, m_rot.y, m_rot.z);

    // Calculate the forward, up, and right vectors from the rotation matrix.
    XMVECTOR forward = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotMatrix);
    XMVECTOR up = XMVector3TransformCoord(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotMatrix);

    // Create the view matrix from the camera's position and orientation.
    return XMMatrixLookAtLH(pos, pos + forward, up);
}
