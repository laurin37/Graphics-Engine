#include <cmath>

#include "../../include/Physics/PhysicsSystem.h"
#include "../../include/Physics/PhysicsBody.h"

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
    // Iterate through all GameObjects in the scene
    for (auto& obj : objects)
    {
        // Check if this GameObject has a PhysicsBody component
        PhysicsBody* physics = obj->GetPhysics();
        if (!physics) continue; // Skip objects without physics
        
        // Skip if the object is kinematic (manually controlled, like Player)
        // Kinematic objects update their own physics in their Update() methods
        if (physics->isKinematic) continue;
        
        // Update the physics for this object
        // Empty ignore list for now - can be extended if needed
        std::vector<GameObject*> ignoreList;
        physics->Update(deltaTime, obj.get(), objects, ignoreList);
    }
}