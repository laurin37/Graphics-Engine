#pragma once

#include "../ComponentManager.h"

namespace ECS {

class MovementSystem {
public:
    void Update(ComponentManager& cm, float deltaTime);
};

} // namespace ECS
