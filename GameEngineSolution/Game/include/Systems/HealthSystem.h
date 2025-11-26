#pragma once

#include "ECS/ComponentManager.h"
#include "ECS/System.h"

class HealthSystem : public ECS::System {
public:
    explicit HealthSystem(ECS::ComponentManager& cm) : ECS::System(cm) {}
    void Update(float deltaTime) override;
};
