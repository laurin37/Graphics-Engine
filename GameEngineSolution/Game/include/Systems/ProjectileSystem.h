#pragma once

#include "ECS/ComponentManager.h"
#include "ECS/System.h"

class ProjectileSystem : public ECS::System {
public:
    explicit ProjectileSystem(ECS::ComponentManager& cm) : ECS::System(cm) {}
    void Update(float deltaTime) override;
};
