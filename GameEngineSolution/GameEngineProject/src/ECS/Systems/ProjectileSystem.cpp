#include "../../../include/ECS/Systems/ProjectileSystem.h"
#include "../../../include/Renderer/Mesh.h"
#include <iostream>
#include <format>

void ProjectileSystem::Update(ECS::ComponentManager& componentManager, float deltaTime) {
    auto projectileArray = componentManager.GetComponentArray<ECS::ProjectileComponent>();
    
    // Iterate backwards to safely remove entities
    for (int i = (int)projectileArray->GetSize() - 1; i >= 0; --i) {
        ECS::Entity entity = projectileArray->GetEntityAtIndex(i);
        ECS::ProjectileComponent& projectile = projectileArray->GetData(entity);

        // Update lifetime
        projectile.lifetime -= deltaTime;
        if (projectile.lifetime <= 0.0f) {
            componentManager.DestroyEntity(entity);
            continue;
        }

        // Update position based on velocity
        if (componentManager.HasComponent<ECS::TransformComponent>(entity)) {
            ECS::TransformComponent& transform = componentManager.GetComponent<ECS::TransformComponent>(entity);
            
            transform.position.x += projectile.velocity.x * projectile.speed * deltaTime;
            transform.position.y += projectile.velocity.y * projectile.speed * deltaTime;
            transform.position.z += projectile.velocity.z * projectile.speed * deltaTime;
        }

        // Simple collision check (Point vs AABB)
        // In a real engine, use the PhysicsSystem for this
        // Simple collision check (Point vs AABB)
        // In a real engine, use the PhysicsSystem for this
        
        bool hit = false;
        ECS::Entity hitEntity = ECS::NULL_ENTITY;

        // 1. Check Colliders (Walls, etc.)
        auto colliderArray = componentManager.GetComponentArray<ECS::ColliderComponent>();
        for (size_t j = 0; j < colliderArray->GetSize(); ++j) {
            ECS::Entity targetEntity = colliderArray->GetEntityAtIndex(j);
            if (targetEntity == entity) continue; // Don't hit self

            if (!componentManager.HasComponent<ECS::TransformComponent>(targetEntity)) continue;
            auto& targetTransform = componentManager.GetComponent<ECS::TransformComponent>(targetEntity);
            auto& collider = colliderArray->GetData(targetEntity);
            
            if (!collider.enabled) continue;

            // Check point vs AABB
            if (componentManager.HasComponent<ECS::TransformComponent>(entity)) {
                auto& projTransform = componentManager.GetComponent<ECS::TransformComponent>(entity);
                
                float minX = targetTransform.position.x + (collider.localAABB.center.x - collider.localAABB.extents.x) * targetTransform.scale.x;
                float maxX = targetTransform.position.x + (collider.localAABB.center.x + collider.localAABB.extents.x) * targetTransform.scale.x;
                float minY = targetTransform.position.y + (collider.localAABB.center.y - collider.localAABB.extents.y) * targetTransform.scale.y;
                float maxY = targetTransform.position.y + (collider.localAABB.center.y + collider.localAABB.extents.y) * targetTransform.scale.y;
                float minZ = targetTransform.position.z + (collider.localAABB.center.z - collider.localAABB.extents.z) * targetTransform.scale.z;
                float maxZ = targetTransform.position.z + (collider.localAABB.center.z + collider.localAABB.extents.z) * targetTransform.scale.z;

                if (projTransform.position.x >= minX && projTransform.position.x <= maxX &&
                    projTransform.position.y >= minY && projTransform.position.y <= maxY &&
                    projTransform.position.z >= minZ && projTransform.position.z <= maxZ) {
                    hit = true;
                    hitEntity = targetEntity;
                    break;
                }
            }
        }

        // 2. Check Health Entities (if not already hit)
        if (!hit) {
            auto healthArray = componentManager.GetComponentArray<ECS::HealthComponent>();
            for (size_t j = 0; j < healthArray->GetSize(); ++j) {
                ECS::Entity targetEntity = healthArray->GetEntityAtIndex(j);
                if (targetEntity == entity) continue; // Don't hit self
                
                // Skip if already checked (has Collider)
                if (componentManager.HasComponent<ECS::ColliderComponent>(targetEntity)) continue;

                if (!componentManager.HasComponent<ECS::TransformComponent>(targetEntity)) continue;
                auto& targetTransform = componentManager.GetComponent<ECS::TransformComponent>(targetEntity);
                
                // Determine collision bounds
                AABB localBounds;
                bool hasBounds = false;

                if (componentManager.HasComponent<ECS::RenderComponent>(targetEntity)) {
                    auto& render = componentManager.GetComponent<ECS::RenderComponent>(targetEntity);
                    if (render.mesh) {
                        localBounds = render.mesh->GetLocalBounds();
                        hasBounds = true;
                    }
                }

                if (!hasBounds) {
                    localBounds.center = { 0.0f, 0.0f, 0.0f };
                    localBounds.extents = { 0.5f, 0.5f, 0.5f }; // Default 1.0 size
                }

                if (componentManager.HasComponent<ECS::TransformComponent>(entity)) {
                    auto& projTransform = componentManager.GetComponent<ECS::TransformComponent>(entity);
                    
                    float minX = targetTransform.position.x + (localBounds.center.x - localBounds.extents.x) * targetTransform.scale.x;
                    float maxX = targetTransform.position.x + (localBounds.center.x + localBounds.extents.x) * targetTransform.scale.x;
                    float minY = targetTransform.position.y + (localBounds.center.y - localBounds.extents.y) * targetTransform.scale.y;
                    float maxY = targetTransform.position.y + (localBounds.center.y + localBounds.extents.y) * targetTransform.scale.y;
                    float minZ = targetTransform.position.z + (localBounds.center.z - localBounds.extents.z) * targetTransform.scale.z;
                    float maxZ = targetTransform.position.z + (localBounds.center.z + localBounds.extents.z) * targetTransform.scale.z;

                    if (projTransform.position.x >= minX && projTransform.position.x <= maxX &&
                        projTransform.position.y >= minY && projTransform.position.y <= maxY &&
                        projTransform.position.z >= minZ && projTransform.position.z <= maxZ) {
                        hit = true;
                        hitEntity = targetEntity;
                        break;
                    }
                }
            }
        }

        if (hit) {
            std::string hitMsg = std::format("Projectile hit Entity {}!", hitEntity);
            std::cout << hitMsg << std::endl;
            
            if (componentManager.HasComponent<ECS::HealthComponent>(hitEntity)) {
                auto& health = componentManager.GetComponent<ECS::HealthComponent>(hitEntity);
                health.currentHealth -= projectile.damage;
            }
            
            componentManager.DestroyEntity(entity); // Destroy projectile
            continue; // Move to next projectile
        }
    }
}
