#pragma once
#include <DirectXMath.h>

class Transform
{
public:
    Transform();

    void SetPosition(float x, float y, float z);
    void SetRotation(float x, float y, float z); // In radians
    void SetScale(float x, float y, float z);

    DirectX::XMFLOAT3 GetPosition() const { return m_pos; }
    DirectX::XMFLOAT3 GetRotation() const { return m_rot; }
    DirectX::XMFLOAT3 GetScale() const { return m_scale; }

    DirectX::XMMATRIX GetWorldMatrix() const;

private:
    DirectX::XMFLOAT3 m_pos;
    DirectX::XMFLOAT3 m_rot;
    DirectX::XMFLOAT3 m_scale;
};
