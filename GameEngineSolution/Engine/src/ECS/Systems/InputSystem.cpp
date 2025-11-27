#include "../../../include/ECS/Systems/InputSystem.h"
#include "../../../include/ECS/Components.h"

namespace ECS {

void InputSystem::Init() {
    // Nothing to init for now
}

void InputSystem::Update(float deltaTime) {
    // Query for entities with InputComponent AND PlayerControllerComponent
    // (Assuming only players are controlled by hardware input)
    auto entities = m_componentManager.QueryEntities<InputComponent, PlayerControllerComponent>();

    for (Entity entity : entities) {
        auto& inputComp = m_componentManager.GetComponent<InputComponent>(entity);
        
        // Reset one-shot actions from previous frame
        inputComp.ResetActions();

        // Map Movement Axes
        inputComp.moveX = 0.0f;
        inputComp.moveZ = 0.0f;

        if (m_input.IsActionDown(Action::MoveForward)) inputComp.moveZ += 1.0f;
        if (m_input.IsActionDown(Action::MoveBackward)) inputComp.moveZ -= 1.0f;
        if (m_input.IsActionDown(Action::MoveRight)) inputComp.moveX += 1.0f;
        if (m_input.IsActionDown(Action::MoveLeft)) inputComp.moveX -= 1.0f;

        // Map Look Axes
        inputComp.lookX = static_cast<float>(m_input.GetMouseDeltaX());
        inputComp.lookY = static_cast<float>(m_input.GetMouseDeltaY());

        // Map Actions
        inputComp.jump = m_input.IsActionDown(Action::Jump);
        inputComp.fire = m_input.IsActionDown(Action::Fire);
        inputComp.altFire = m_input.IsActionDown(Action::AltFire);
        inputComp.reload = m_input.IsActionDown(Action::Reload);
        
        // Normalize movement vector if diagonal
        if (inputComp.moveX != 0.0f || inputComp.moveZ != 0.0f) {
            float len = std::sqrt(inputComp.moveX * inputComp.moveX + inputComp.moveZ * inputComp.moveZ);
            inputComp.moveX /= len;
            inputComp.moveZ /= len;
        }
    }
}

} // namespace ECS
