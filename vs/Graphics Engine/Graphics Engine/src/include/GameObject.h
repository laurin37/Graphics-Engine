#pragma once
#include <DirectXMath.h>
#include <memory>
#include <string> // Required for std::wstring
#include "Mesh.h"
#include "Material.h"
#include "Collision.h"
#include "Transform.h" // Include the new Transform class

class GameObject
{
public:
    GameObject(Mesh* mesh, std::shared_ptr<Material> material);
    virtual ~GameObject() = default; // Make destructor virtual to allow dynamic_cast

    void SetPosition(float x, float y, float z) { m_transform.SetPosition(x, y, z); }
    void SetRotation(float x, float y, float z) { m_transform.SetRotation(x, y, z); } // In radians
    void SetScale(float x, float y, float z) { m_transform.SetScale(x, y, z); }

    DirectX::XMFLOAT3 GetPosition() const { return m_transform.GetPosition(); }
    DirectX::XMFLOAT3 GetRotation() const { return m_transform.GetRotation(); }
    DirectX::XMMATRIX GetWorldMatrix() const { return m_transform.GetWorldMatrix(); }
    void Draw(ID3D11DeviceContext* context, ID3D11Buffer* psMaterialConstantBuffer) const;

    void SetBoundingBox(const AABB& box);
    AABB GetWorldBoundingBox() const;

    // New methods for naming
    void SetName(const std::wstring& name) { m_name = name; }
    const std::wstring& GetName() const { return m_name; }

    // New methods for setting mesh and material
    void SetMesh(Mesh* mesh);
    void SetMaterial(std::shared_ptr<Material> material);

protected: // Changed to protected to allow Bullet/HealthObject to access m_mesh/m_material directly
    Mesh* m_mesh;
    std::shared_ptr<Material> m_material;
    Transform m_transform; // Encapsulate position, rotation, scale

private:
    AABB m_boundingBox;
    std::wstring m_name; // Object name
};