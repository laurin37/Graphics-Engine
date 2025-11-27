#pragma once

#include "../System.h"
#include "../SystemPhase.h"
#include "../../Input/Input.h"

namespace ECS {

class InputSystem : public System {
public:
    InputSystem(ComponentManager& cm, Input& input) 
        : System(cm), m_input(input) {}

    void Init() override;
    void Update(float deltaTime) override;

    SystemPhase GetPhase() const override { return SystemPhase::PreUpdate; }
    bool CanParallelize() const override { return false; } // Reads hardware input (not thread safe usually)

private:
    Input& m_input;
};

} // namespace ECS
