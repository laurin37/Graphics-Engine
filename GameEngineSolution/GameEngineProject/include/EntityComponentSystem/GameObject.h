#pragma once

#include <DirectXMath.h>
#include <memory>
#include <string>

#include "../Renderer/Mesh.h"
#include "../Renderer/Material.h"
#include "../Physics/Collision.h"
#include "../Utils/Transform.h"
#include "../Physics/Collider.h"

// Forward declarations
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
    DirectX::XMFLOAT3 GetScale() const { return m_transform.GetScale(); }
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
    
    // Mark as kinematic (manually controlled, skips PhysicsSystem updates)
    void SetKinematic(bool kinematic);
    
    // Optional Collider Component
    Collider* GetCollider() const { return m_collider; }
    void SetCollider(Collider* collider) { m_collider = collider; }
    
    // Auto-generate collider from current mesh
    void GenerateCollider(ColliderType type = ColliderType::AABB);

protected:
    Mesh* m_mesh;
    std::shared_ptr<Material> m_material;
    Transform m_transform;
    PhysicsBody* m_physics = nullptr;
    Collider* m_collider = nullptr;

private:
    AABB m_boundingBox; // Legacy - kept for backward compatibility
    std::wstring m_name;
};