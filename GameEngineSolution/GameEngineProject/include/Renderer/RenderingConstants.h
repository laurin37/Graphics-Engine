#pragma once

// ==================================================
// Rendering Engine Constants
// ==================================================
// Centralized rendering configuration values used by
// the Renderer, Graphics, and shader systems.

namespace RenderingConstants
{
    // ===== Shadow Mapping =====
    constexpr int SHADOW_MAP_SIZE = 2048;               // Shadow map resolution (width/height)
    constexpr int SHADOW_DEPTH_BIAS = 10000;            // Depth bias to reduce shadow acne
    constexpr float SHADOW_SLOPE_BIAS = 1.0f;           // Slope-scaled depth bias
    constexpr float SHADOW_ORTHO_SIZE = 40.0f;          // Orthographic projection size for shadows
    constexpr float SHADOW_NEAR_PLANE = 0.1f;           // Shadow frustum near plane
    constexpr float SHADOW_FAR_PLANE = 100.0f;          // Shadow frustum far plane
    
    // ===== Lighting =====
    constexpr int MAX_POINT_LIGHTS = 4;                 // Maximum simultaneous point lights
    
    // ===== Camera =====
    constexpr float DEFAULT_FOV = 3.14159265f / 4.0f;   // 45 degrees field of view (radians)
    constexpr float CAMERA_NEAR_PLANE = 0.1f;           // Camera frustum near plane
    constexpr float CAMERA_FAR_PLANE = 100.0f;          // Camera frustum far plane
    
    // ===== Default Screen Resolution =====
    // NOTE: These are defaults used in Scene for UI calculations
    // Actual resolution is determined by Window/Graphics system
    constexpr float DEFAULT_SCREEN_WIDTH = 1280.0f;
    constexpr float DEFAULT_SCREEN_HEIGHT = 720.0f;
    
    // ===== Texture Filtering =====
    constexpr int MAX_ANISOTROPY = 16;                  // Maximum anisotropic filtering samples
}
