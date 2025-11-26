#pragma once

#include "../ComponentManager.h"
#include "../System.h"
#include "../../Renderer/Renderer.h"
#include "../../Renderer/Camera.h"

namespace ECS {

// ========================================
// RenderSystem
// Handles rendering for entities with
// RenderComponent + TransformComponent
// ========================================
class RenderSystem : public System {
public:
    explicit RenderSystem(ComponentManager& cm) : System(cm) {}
    
    // Render all renderable entities
    void Render(Renderer* renderer, Camera& camera);
    
    // Debug rendering (bounding boxes)
    void RenderDebug(Renderer* renderer, Camera& camera);
};

} // namespace ECS
