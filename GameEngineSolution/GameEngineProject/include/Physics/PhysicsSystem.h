#pragma once

#include <vector>

#include "../EntityComponentSystem/GameObject.h"

class PhysicsSystem
{
public:
    void Update(std::vector<std::unique_ptr<GameObject>>& objects, float deltaTime);
    static bool AABBIntersects(const AABB& boxA, const AABB& boxB);
};