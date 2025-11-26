#pragma once

#include "../ECS/ComponentManager.h"
#include "SimpleFont.h"

// Forward declarations
class UIRenderer;
class Renderer;

// ========================================
// DebugUIRenderer
// Handles all debug UI text rendering
// Can be easily toggled on/off
// ========================================
class DebugUIRenderer
{
public:
    DebugUIRenderer();
    ~DebugUIRenderer() = default;

    // Enable/disable debug UI
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    bool IsEnabled() const { return m_enabled; }
    void Toggle() { m_enabled = !m_enabled; }

    // Render all debug UI elements
    void Render(
        UIRenderer* uiRenderer,
        const SimpleFont& font,
        int fps,
        bool bloomEnabled,
        bool debugCollisionEnabled,
        ECS::ComponentManager& componentManager,
        ECS::Entity activeCamera = ECS::NULL_ENTITY
    );

    // Update timers
    void Update(float deltaTime);

    // Static message logging
    static void AddMessage(const std::string& message, float duration);

private:
    bool m_enabled = true;

    struct DebugMessage {
        std::string text;
        float duration;
        float timeRemaining;
    };

    static std::vector<DebugMessage> s_messages;
};
