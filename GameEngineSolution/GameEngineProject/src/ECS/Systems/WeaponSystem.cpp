#define NOMINMAX
#include "../../../include/ECS/Systems/WeaponSystem.h"
#include "../../../include/UI/DebugUIRenderer.h"
#include "../../../include/Renderer/Mesh.h"
#include <iostream>
#include <format>
#include <cmath>
#include <algorithm> // For std::max

void WeaponSystem::Update(float deltaTime) {
    auto weaponArray = m_componentManager.GetComponentArray<ECS::WeaponComponent>();
    
    for (size_t i = 0; i < weaponArray->GetSize(); ++i) {
        ECS::Entity entity = weaponArray->GetEntityAtIndex(i);
        ECS::WeaponComponent& weapon = weaponArray->GetData(entity);

        // Cooldown management
        if (weapon.timeSinceLastShot < weapon.fireRate) {
            weapon.timeSinceLastShot += deltaTime;
        }

        // Check if this entity is controlled by player (has PlayerController)
        // Only player weapons respond to input for now
        if (m_componentManager.HasComponent<ECS::PlayerControllerComponent>(entity)) {
            // TODO: Implement proper mouse button support in Input class
            // For now, use VK_LBUTTON for both. 
            // Note: IsKeyDown is "held down", so semi-auto might fire repeatedly if fireRate is low.
            bool fireInput = m_input.IsKeyDown(VK_LBUTTON);
            bool altFireInput = m_input.IsKeyDown(VK_RBUTTON);
            
            if (weapon.timeSinceLastShot >= weapon.fireRate && weapon.currentAmmo > 0) {
                if (m_componentManager.HasComponent<ECS::TransformComponent>(entity)) {
                    ECS::TransformComponent& transform = m_componentManager.GetComponent<ECS::TransformComponent>(entity);
                    
                    if (fireInput) {
                        FireWeapon(entity, weapon, transform);
                    } else if (altFireInput && m_projectileMesh && m_projectileMaterial) {
                        FireProjectile(entity, transform);
                        weapon.timeSinceLastShot = 0.0f;
                        // weapon.currentAmmo--; // Optional: consume ammo for projectiles too?
                    }
                }
            }
        }
    }
}

void WeaponSystem::FireProjectile(ECS::Entity entity, ECS::TransformComponent& transform) {
    // Create projectile entity
    ECS::Entity projectile = m_componentManager.CreateEntity();
    
    // Calculate spawn position (same as ray origin)
    DirectX::XMFLOAT3 spawnPos = transform.position;
    if (m_componentManager.HasComponent<ECS::PlayerControllerComponent>(entity)) {
        auto& pc = m_componentManager.GetComponent<ECS::PlayerControllerComponent>(entity);
        spawnPos.y += pc.cameraHeight;
    }

    // Calculate direction
    float pitch = transform.rotation.x;
    float yaw = transform.rotation.y;
    if (m_componentManager.HasComponent<ECS::PlayerControllerComponent>(entity)) {
        pitch = m_componentManager.GetComponent<ECS::PlayerControllerComponent>(entity).viewPitch;
    }

    DirectX::XMFLOAT3 dir;
    dir.x = cos(pitch) * sin(yaw);
    dir.y = -sin(pitch);
    dir.z = cos(pitch) * cos(yaw);

    // Normalize
    float len = sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
    dir.x /= len; dir.y /= len; dir.z /= len;

    // Offset spawn position slightly forward to avoid self-collision
    spawnPos.x += dir.x * 1.0f;
    spawnPos.y += dir.y * 1.0f;
    spawnPos.z += dir.z * 1.0f;

    // Add components
    m_componentManager.AddComponent(projectile, ECS::TransformComponent{ spawnPos, {0,0,0}, {0.5f, 0.5f, 0.5f} });
    m_componentManager.AddComponent(projectile, ECS::RenderComponent{ m_projectileMesh, m_projectileMaterial });
    
    ECS::PhysicsComponent physics;
    physics.useGravity = false; // "not effected from gravity"
    physics.mass = 1.0f;
    physics.velocity = { dir.x * 20.0f, dir.y * 20.0f, dir.z * 20.0f }; // Speed 20
    physics.checkCollisions = false; // Handled by ProjectileSystem manually for now
    m_componentManager.AddComponent(projectile, physics);

    ECS::ProjectileComponent projComp;
    projComp.damage = 20.0f;
    projComp.lifetime = 5.0f;
    projComp.speed = 20.0f;
    projComp.velocity = physics.velocity; // Redundant but used by ProjectileSystem
    m_componentManager.AddComponent(projectile, projComp);

    // std::cout << "Fired Projectile!" << std::endl;
}

void WeaponSystem::FireWeapon(ECS::Entity entity, ECS::WeaponComponent& weapon, ECS::TransformComponent& transform) {
    weapon.timeSinceLastShot = 0.0f;
    weapon.currentAmmo--;

    std::string bangMsg = std::format("BANG! Ammo: {}/{}", weapon.currentAmmo, weapon.maxAmmo);
    DebugUIRenderer::AddMessage(bangMsg, 1.0f);

    // Calculate ray origin and direction
    // Origin is player position + camera offset (if any)
    DirectX::XMFLOAT3 rayOrigin = transform.position;
    
    // Adjust for camera height if available
    if (m_componentManager.HasComponent<ECS::PlayerControllerComponent>(entity)) {
        auto& pc = m_componentManager.GetComponent<ECS::PlayerControllerComponent>(entity);
        rayOrigin.y += pc.cameraHeight;
    }

    // Direction based on rotation (Pitch/Yaw)
    // Assuming rotation.x is pitch, rotation.y is yaw
    float pitch = transform.rotation.x;
    float yaw = transform.rotation.y;
    
    // If player controller exists, use its view pitch
    if (m_componentManager.HasComponent<ECS::PlayerControllerComponent>(entity)) {
        pitch = m_componentManager.GetComponent<ECS::PlayerControllerComponent>(entity).viewPitch;
    }

    DirectX::XMFLOAT3 rayDir;
    rayDir.x = cos(pitch) * sin(yaw);
    rayDir.y = -sin(pitch); // Invert pitch because +Pitch is looking down (negative Y)
    rayDir.z = cos(pitch) * cos(yaw);

    // Normalize direction
    float length = sqrt(rayDir.x * rayDir.x + rayDir.y * rayDir.y + rayDir.z * rayDir.z);
    rayDir.x /= length;
    rayDir.y /= length;
    rayDir.z /= length;

    // Raycast against all entities with Health and Collider
    // This is a naive O(N) raycast. In a real engine, use a spatial partition (Octree/BVH).
    
    ECS::Entity hitEntity = ECS::NULL_ENTITY;
    float minDistance = weapon.range;

    // Iterate over ColliderComponent array (Walls, Props, etc.)
    auto colliderArray = m_componentManager.GetComponentArray<ECS::ColliderComponent>();
    for (size_t i = 0; i < colliderArray->GetSize(); ++i) {
        ECS::Entity targetEntity = colliderArray->GetEntityAtIndex(i);
        if (targetEntity == entity) continue; // Don't hit self

        if (!m_componentManager.HasComponent<ECS::TransformComponent>(targetEntity)) continue;
        auto& targetTransform = m_componentManager.GetComponent<ECS::TransformComponent>(targetEntity);
        auto& collider = colliderArray->GetData(targetEntity);
        
        if (!collider.enabled) continue;

        // Calculate World AABB (Axis Aligned approximation)
        DirectX::XMFLOAT3 minBox, maxBox;
        
        // Center in world space
        float cx = targetTransform.position.x + collider.localAABB.center.x * targetTransform.scale.x;
        float cy = targetTransform.position.y + collider.localAABB.center.y * targetTransform.scale.y;
        float cz = targetTransform.position.z + collider.localAABB.center.z * targetTransform.scale.z;
        
        // Extents in world space (abs scale to handle negative scale)
        float ex = collider.localAABB.extents.x * std::abs(targetTransform.scale.x);
        float ey = collider.localAABB.extents.y * std::abs(targetTransform.scale.y);
        float ez = collider.localAABB.extents.z * std::abs(targetTransform.scale.z);

        minBox = { cx - ex, cy - ey, cz - ez };
        maxBox = { cx + ex, cy + ey, cz + ez };

        float t = 0.0f;
        if (RayAABBIntersect(rayOrigin, rayDir, minBox, maxBox, t)) {
            if (t < minDistance) {
                minDistance = t;
                hitEntity = targetEntity;
            }
        }
    }

    // Iterate over HealthComponent array (Enemies without colliders)
    auto healthArray = m_componentManager.GetComponentArray<ECS::HealthComponent>();
    for (size_t i = 0; i < healthArray->GetSize(); ++i) {
        ECS::Entity targetEntity = healthArray->GetEntityAtIndex(i);
        if (targetEntity == entity) continue; // Don't hit self
        
        // Skip if already checked (has Collider)
        if (m_componentManager.HasComponent<ECS::ColliderComponent>(targetEntity)) continue;

        if (!m_componentManager.HasComponent<ECS::TransformComponent>(targetEntity)) continue;
        auto& targetTransform = m_componentManager.GetComponent<ECS::TransformComponent>(targetEntity);

        // Determine collision bounds (Render or Default)
        AABB localBounds;
        bool hasBounds = false;

        if (m_componentManager.HasComponent<ECS::RenderComponent>(targetEntity)) {
            auto& render = m_componentManager.GetComponent<ECS::RenderComponent>(targetEntity);
            if (render.mesh) {
                localBounds = render.mesh->GetLocalBounds();
                hasBounds = true;
            }
        }

        if (!hasBounds) {
            localBounds.center = { 0.0f, 0.0f, 0.0f };
            localBounds.extents = { 0.5f, 0.5f, 0.5f }; // Default 1.0 size
        }

        // Calculate World AABB
        DirectX::XMFLOAT3 minBox, maxBox;
        
        float cx = targetTransform.position.x + localBounds.center.x * targetTransform.scale.x;
        float cy = targetTransform.position.y + localBounds.center.y * targetTransform.scale.y;
        float cz = targetTransform.position.z + localBounds.center.z * targetTransform.scale.z;
        
        float ex = localBounds.extents.x * std::abs(targetTransform.scale.x);
        float ey = localBounds.extents.y * std::abs(targetTransform.scale.y);
        float ez = localBounds.extents.z * std::abs(targetTransform.scale.z);

        minBox = { cx - ex, cy - ey, cz - ez };
        maxBox = { cx + ex, cy + ey, cz + ez };

        float t = 0.0f;
        if (RayAABBIntersect(rayOrigin, rayDir, minBox, maxBox, t)) {
            if (t < minDistance) {
                minDistance = t;
                hitEntity = targetEntity;
            }
        }
    }

    if (hitEntity != ECS::NULL_ENTITY) {
        if (m_componentManager.HasComponent<ECS::HealthComponent>(hitEntity)) {
            auto& health = m_componentManager.GetComponent<ECS::HealthComponent>(hitEntity);
            health.currentHealth -= weapon.damage;
        }
    }
}

bool WeaponSystem::RayAABBIntersect(
    const DirectX::XMFLOAT3& rayOrigin, 
    const DirectX::XMFLOAT3& rayDir, 
    const DirectX::XMFLOAT3& boxMin, 
    const DirectX::XMFLOAT3& boxMax, 
    float& t
) {
    float tmin = 0.0f;
    float tmax = FLT_MAX;

    // X axis
    if (std::abs(rayDir.x) < 1e-6f) {
        // Ray is parallel to slab. No hit if origin not within slab
        if (rayOrigin.x < boxMin.x || rayOrigin.x > boxMax.x) return false;
    } else {
        float ood = 1.0f / rayDir.x;
        float t1 = (boxMin.x - rayOrigin.x) * ood;
        float t2 = (boxMax.x - rayOrigin.x) * ood;
        if (t1 > t2) std::swap(t1, t2);
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return false;
    }

    // Y axis
    if (std::abs(rayDir.y) < 1e-6f) {
        if (rayOrigin.y < boxMin.y || rayOrigin.y > boxMax.y) return false;
    } else {
        float ood = 1.0f / rayDir.y;
        float t1 = (boxMin.y - rayOrigin.y) * ood;
        float t2 = (boxMax.y - rayOrigin.y) * ood;
        if (t1 > t2) std::swap(t1, t2);
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return false;
    }

    // Z axis
    if (std::abs(rayDir.z) < 1e-6f) {
        if (rayOrigin.z < boxMin.z || rayOrigin.z > boxMax.z) return false;
    } else {
        float ood = 1.0f / rayDir.z;
        float t1 = (boxMin.z - rayOrigin.z) * ood;
        float t2 = (boxMax.z - rayOrigin.z) * ood;
        if (t1 > t2) std::swap(t1, t2);
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return false;
    }

    t = tmin;
    return true;
}
