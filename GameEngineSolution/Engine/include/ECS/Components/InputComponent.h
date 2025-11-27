#pragma once

#include <DirectXMath.h>

namespace ECS {

struct InputComponent {
    // Movement axes (range -1.0 to 1.0)
    float moveX = 0.0f;
    float moveY = 0.0f; // Usually forward/backward
    float moveZ = 0.0f;
    
    // Look axes (mouse delta)
    float lookX = 0.0f;
    float lookY = 0.0f;
    
    // Actions
    bool jump = false;
    bool fire = false;
    bool altFire = false;
    bool reload = false;
    bool sprint = false;
    bool crouch = false;
    
    // Helper to reset one-shot actions (optional, handled by system)
    void ResetActions() {
        jump = false;
        fire = false;
        altFire = false;
        reload = false;
    }
};

} // namespace ECS
