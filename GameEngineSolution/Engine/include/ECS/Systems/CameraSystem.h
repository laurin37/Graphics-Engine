#pragma once

#include "../ComponentManager.h"
#include "../System.h"
#include <DirectXMath.h>

namespace ECS {

// ========================================
// CameraSystem
// Updates camera view and projection matrices
// ========================================
class CameraSystem : public System {
public:
    explicit CameraSystem(ComponentManager& cm) : System(cm) {}

    // Update all camera view and projection matrices
    void Update(float deltaTime) override;
    
    // Get active camera's matrices  (returns false if no active camera)
    bool GetActiveCamera(DirectX::XMMATRIX& viewOut, DirectX::XMMATRIX& projOut);

    // Get the active camera entity ID
    Entity GetActiveCameraEntity();
};

}
