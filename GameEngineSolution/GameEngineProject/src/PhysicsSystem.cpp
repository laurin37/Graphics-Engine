#include "include/PhysicsSystem.h"
#include <cmath>

bool PhysicsSystem::AABBIntersects(const AABB& boxA, const AABB& boxB)
{
    // Logic adapted from GameObject::GetWorldBoundingBox
    return (boxA.center.x - boxA.extents.x <= boxB.center.x + boxB.extents.x &&
        boxA.center.x + boxA.extents.x >= boxB.center.x - boxB.extents.x) &&
        (boxA.center.y - boxA.extents.y <= boxB.center.y + boxB.extents.y &&
            boxA.center.y + boxA.extents.y >= boxB.center.y - boxB.extents.y) &&
        (boxA.center.z - boxA.extents.z <= boxB.center.z + boxB.extents.z &&
            boxA.center.z + boxA.extents.z >= boxB.center.z - boxB.extents.z);
}

void PhysicsSystem::Update(std::vector<std::unique_ptr<GameObject>>& objects, float deltaTime)
{
    //     
    for (size_t i = 0; i < objects.size(); ++i)
    {
        // Skip static objects (like the floor or pillars) if they are marked as static
        // For now, we assume index 0 is the floor and we don't move it.
        if (i == 0) continue;

        auto& obj = objects[i];

        // 1. Apply Gravity (If the object has physical properties)
        // You might want to add a 'velocity' member to GameObject later.
        // For now, we will just handle collision checks here.

        // 2. Collision Detection
        for (size_t j = 0; j < objects.size(); ++j)
        {
            if (i == j) continue;

            if (AABBIntersects(obj->GetWorldBoundingBox(), objects[j]->GetWorldBoundingBox()))
            {
                // Simple resolution: Move back to previous position
                // Note: This requires GameObject to store 'prevPosition', 
                // or we pass it in.
            }
        }
    }
}