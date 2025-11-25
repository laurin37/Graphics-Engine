#include "../../../include/ECS/Systems/ECSMovementSystem.h"
#include <cmath>

namespace ECS {

void MovementSystem::Update(ComponentManager& cm, float deltaTime) {
    // --- Handle Rotation ---
    auto rotateArray = cm.GetComponentArray<RotateComponent>();
    auto& rotateVec = rotateArray->GetComponentArray();
    
    for (size_t i = 0; i < rotateVec.size(); ++i) {
        Entity entity = rotateArray->GetEntityAtIndex(i);
        RotateComponent& rotate = rotateVec[i];
        
        if (!cm.HasComponent<TransformComponent>(entity)) continue;
        TransformComponent& transform = cm.GetComponent<TransformComponent>(entity);
        
        // Simple Euler integration for rotation
        transform.rotation.x += rotate.axis.x * rotate.speed * deltaTime;
        transform.rotation.y += rotate.axis.y * rotate.speed * deltaTime;
        transform.rotation.z += rotate.axis.z * rotate.speed * deltaTime;
    }

    // --- Handle Orbiting ---
    auto orbitArray = cm.GetComponentArray<OrbitComponent>();
    auto& orbitVec = orbitArray->GetComponentArray();
    
    for (size_t i = 0; i < orbitVec.size(); ++i) {
        Entity entity = orbitArray->GetEntityAtIndex(i);
        OrbitComponent& orbit = orbitVec[i];
        
        if (!cm.HasComponent<TransformComponent>(entity)) continue;
        TransformComponent& transform = cm.GetComponent<TransformComponent>(entity);
        
        // Update angle
        orbit.angle += orbit.speed * deltaTime;
        
        // Calculate new position based on orbit
        // Assuming orbit around Y axis for simplicity, but using axis would be better
        // For now, replicating the scene logic: x = cos(angle)*r, z = sin(angle)*r
        
        float x = orbit.center.x + cosf(orbit.angle) * orbit.radius;
        float z = orbit.center.z + sinf(orbit.angle) * orbit.radius;
        float y = orbit.center.y + sinf(orbit.angle * 2.0f) * 0.3f; // Slight bobbing
        
        transform.position = { x, y, z };
    }
}

} // namespace ECS
