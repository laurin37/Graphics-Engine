#pragma once

#include "ECS/ComponentManager.h"
#include "ECS/System.h"
#include "Input/Input.h"

class WeaponSystem : public ECS::System {
public:
    WeaponSystem(ECS::ComponentManager& cm, Input& input) 
        : ECS::System(cm), m_input(input) {}

    void SetProjectileAssets(Mesh* mesh, std::shared_ptr<Material> material) {
        m_projectileMesh = mesh;
        m_projectileMaterial = material;
    }

    void Update(float deltaTime) override;

private:
    Input& m_input;
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
