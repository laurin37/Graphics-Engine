#pragma once
#include <DirectXMath.h>

class Camera
{
public:
    Camera();
    ~Camera() = default;

    void SetPosition(float x, float y, float z);
    void SetRotation(float x, float y, float z); // In radians

    DirectX::XMVECTOR GetPosition() const;
    DirectX::XMFLOAT3 GetPositionFloat3() const;
    DirectX::XMVECTOR GetRotation() const;

    void AdjustPosition(float x, float y, float z);
    void AdjustRotation(float x, float y, float z);

    DirectX::XMMATRIX GetViewMatrix() const;

private:
    DirectX::XMFLOAT3 m_pos;
    DirectX::XMFLOAT3 m_rot;
};
