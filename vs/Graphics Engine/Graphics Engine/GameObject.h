#pragma once
#include <DirectXMath.h>
#include <memory>
#include "Mesh.h"
#include "Material.h"

class GameObject
{
public:
    GameObject(Mesh* mesh, std::shared_ptr<Material> material);
    ~GameObject() = default;

    void SetPosition(float x, float y, float z);
    void SetRotation(float x, float y, float z); // In radians
    void SetScale(float x, float y, float z);

    DirectX::XMMATRIX GetWorldMatrix() const;
    void Draw(ID3D11DeviceContext* context, ID3D11Buffer* psMaterialConstantBuffer) const;

private:
    DirectX::XMFLOAT3 m_pos;
    DirectX::XMFLOAT3 m_rot;
    DirectX::XMFLOAT3 m_scale;
    Mesh* m_pMesh;
    std::shared_ptr<Material> m_material;
};
