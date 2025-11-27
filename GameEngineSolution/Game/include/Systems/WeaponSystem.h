#pragma once

#include "ECS/ComponentManager.h"
#include "ECS/System.h"
#include "ECS/System.h"

namespace ECS { class PhysicsSystem; }

class WeaponSystem : public ECS::System {
public:
    explicit WeaponSystem(ECS::ComponentManager& cm) 
        : ECS::System(cm) {}

    void SetProjectileAssets(Mesh* mesh, std::shared_ptr<Material> material) {
        m_projectileMesh = mesh;
        m_projectileMaterial = material;
    }

    void SetPhysicsSystem(ECS::PhysicsSystem* physicsSystem) {
        m_physicsSystem = physicsSystem;
    }

    void Update(float deltaTime) override;

private:
    ECS::PhysicsSystem* m_physicsSystem = nullptr;
    Mesh* m_projectileMesh = nullptr;
    std::shared_ptr<Material> m_projectileMaterial = nullptr;

    void FireWeapon(ECS::Entity entity, ECS::WeaponComponent& weapon, ECS::TransformComponent& transform);
    void FireProjectile(ECS::Entity entity, ECS::TransformComponent& transform);
    
    // Simple ray-AABB intersection for hit detection
    bool RayAABBIntersect(
        const DirectX::XMFLOAT3& rayOrigin, 
        const DirectX::XMFLOAT3& rayDir, 
        const DirectX::XMFLOAT3& boxMin, 
        const DirectX::XMFLOAT3& boxMax, 
        float& t
    );
};
