#pragma once

#include "../ComponentManager.h"
#include <DirectXMath.h>

namespace ECS {

// ========================================
// CameraSystem
// Updates camera view and projection matrices
// ========================================
class CameraSystem {
public:
    // Update all camera view and projection matrices
    void Update(ComponentManager& cm);
    
    // Get active camera's matrices  (returns false if no active camera)
    bool GetActiveCamera(ComponentManager& cm, 
                         DirectX::XMMATRIX& viewOut, 
                         DirectX::XMMATRIX& projOut);
};

}
