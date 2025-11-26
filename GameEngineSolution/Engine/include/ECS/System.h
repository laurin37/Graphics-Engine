#pragma once

#include "ComponentManager.h"

namespace ECS {

class System {
public:
    explicit System(ComponentManager& componentManager) 
        : m_componentManager(componentManager) {}
    
    virtual ~System() = default;

    virtual void Init() {}
    virtual void Update(float deltaTime) {}

protected:
    ComponentManager& m_componentManager;
};

} // namespace ECS
