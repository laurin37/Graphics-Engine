#include "../../../include/ECS/Systems/ECSMovementSystem.h"
#include <cmath>

namespace ECS {

void MovementSystem::Update(ComponentManager& cm, float deltaTime) {
    // --- Handle Rotation ---
    auto rotateEntities = cm.GetEntitiesWithRotateAndTransform();
    for (Entity entity : rotateEntities) {
        RotateComponent* rotate = cm.GetRotate(entity);
        TransformComponent* transform = cm.GetTransform(entity);
        
        if (rotate && transform) {
            // Simple Euler integration for rotation
            // Note: This is a simplification. For proper 3D rotation we should use Quaternions,
            // but for this demo, adding to Euler angles works for simple axis rotation.
            transform->rotation.x += rotate->axis.x * rotate->speed * deltaTime;
            transform->rotation.y += rotate->axis.y * rotate->speed * deltaTime;
            transform->rotation.z += rotate->axis.z * rotate->speed * deltaTime;
        }
    }

    // --- Handle Orbiting ---
    auto orbitEntities = cm.GetEntitiesWithOrbitAndTransform();
    for (Entity entity : orbitEntities) {
        OrbitComponent* orbit = cm.GetOrbit(entity);
        TransformComponent* transform = cm.GetTransform(entity);
        
        if (orbit && transform) {
            // Update angle
            orbit->angle += orbit->speed * deltaTime;
            
            // Calculate new position based on orbit
            // Assuming orbit around Y axis for simplicity, but using axis would be better
            // For now, replicating the scene logic: x = cos(angle)*r, z = sin(angle)*r
            
            float x = orbit->center.x + cosf(orbit->angle) * orbit->radius;
            float z = orbit->center.z + sinf(orbit->angle) * orbit->radius;
            float y = orbit->center.y + sinf(orbit->angle * 2.0f) * 0.3f; // Slight bobbing
            
            transform->position = { x, y, z };
        }
    }
}

} // namespace ECS
