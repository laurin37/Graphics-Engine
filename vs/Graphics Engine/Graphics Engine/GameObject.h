#pragma once
#include <DirectXMath.h>
#include "Mesh.h"

class GameObject
{
public:
    GameObject(Mesh* mesh);
    ~GameObject() = default;

    void SetPosition(float x, float y, float z);
    void SetRotation(float x, float y, float z); // In radians
    void SetScale(float x, float y, float z);

    DirectX::XMMATRIX GetWorldMatrix() const;
    void Draw(ID3D11DeviceContext* context) const;

private:
    DirectX::XMFLOAT3 m_pos;
    DirectX::XMFLOAT3 m_rot;
    DirectX::XMFLOAT3 m_scale;
    Mesh* m_pMesh; // Non-owning pointer to a shared mesh resource
};
