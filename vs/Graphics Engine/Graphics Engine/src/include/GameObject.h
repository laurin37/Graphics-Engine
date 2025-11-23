#pragma once
#include <DirectXMath.h>
#include <memory>
#include <string>
#include "Mesh.h"
#include "Material.h"
#include "Collision.h"
#include "Transform.h"

// Forward declaration
class PhysicsBody;

class GameObject
{
public:
    GameObject(Mesh* mesh, std::shared_ptr<Material> material);
    virtual ~GameObject() = default;

    void SetPosition(float x, float y, float z) { m_transform.SetPosition(x, y, z); }
    void SetRotation(float x, float y, float z) { m_transform.SetRotation(x, y, z); }
    void SetScale(float x, float y, float z) { m_transform.SetScale(x, y, z); }

    DirectX::XMFLOAT3 GetPosition() const { return m_transform.GetPosition(); }
    DirectX::XMFLOAT3 GetRotation() const { return m_transform.GetRotation(); }
    DirectX::XMMATRIX GetWorldMatrix() const { return m_transform.GetWorldMatrix(); }
    
    void Draw(ID3D11DeviceContext* context, ID3D11Buffer* psMaterialConstantBuffer) const;

    void SetBoundingBox(const AABB& box);
    AABB GetWorldBoundingBox() const;

    void SetName(const std::wstring& name) { m_name = name; }
    const std::wstring& GetName() const { return m_name; }

    void SetMesh(Mesh* mesh);
    void SetMaterial(std::shared_ptr<Material> material);
    
    // Optional Physics Component
    PhysicsBody* GetPhysics() const { return m_physics; }
    void SetPhysics(PhysicsBody* physics) { m_physics = physics; }

protected:
    Mesh* m_mesh;
    std::shared_ptr<Material> m_material;
    Transform m_transform;
    PhysicsBody* m_physics = nullptr; // Optional physics component

private:
    AABB m_boundingBox;
    std::wstring m_name;
};