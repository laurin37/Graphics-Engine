#pragma once
#include "GameObject.h"
#include <vector>

class PhysicsSystem
{
public:
    void Update(std::vector<std::unique_ptr<GameObject>>& objects, float deltaTime);
    static bool AABBIntersects(const AABB& boxA, const AABB& boxB);

private:
    const float GRAVITY = -9.81f;

    // Helper to resolve collision between two specific objects
    void ResolveCollision(GameObject& dynamicObj, GameObject& staticObj);
};