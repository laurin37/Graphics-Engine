#pragma once

#include "../ComponentManager.h"
#include "../System.h"

namespace ECS {

class MovementSystem : public System {
public:
    explicit MovementSystem(ComponentManager& cm) : System(cm) {}
    void Update(float deltaTime) override;
};

} // namespace ECS
