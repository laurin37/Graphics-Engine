#pragma once

// ==================================================
// Physics Engine Constants
// ==================================================
// Centralized physics simulation parameters used throughout
// the PhysicsBody and collision systems.

namespace PhysicsConstants
{
    // ===== Gravity =====
    constexpr float DEFAULT_GRAVITY = -9.81f;           // Standard earth gravity (m/s²)
    constexpr float PLAYER_GRAVITY = -15.0f;            // Stronger gravity for responsive player feel
    constexpr float MAX_FALL_SPEED = -15.0f;            // Terminal velocity (m/s)

    // ===== Physics Body Defaults =====
    constexpr float DEFAULT_MASS = 1.0f;                // Default object mass (kg)
    constexpr float DEFAULT_DRAG = 0.0f;                // Air resistance coefficient
    
    // ===== Collision Detection =====
    constexpr float COLLISION_SKIN_WIDTH = 0.005f;      // Collision margin to prevent tunneling (m)
    constexpr float STANDING_TOLERANCE = 0.05f;         // Y-distance to consider "standing on" surface (m) - reduced for tighter collision
    constexpr float GROUND_PROBE_DISTANCE = 0.015f;     // How far down to check for ground (skin * 3)
    
    // ===== Safety Limits =====
    constexpr float MIN_DELTA_TIME = 1.0f / 240.0f;     // Minimum physics timestep (240 FPS cap)
    constexpr float MAX_DELTA_TIME = 1.0f / 30.0f;      // Maximum physics timestep (30 FPS floor)
    constexpr float RESPAWN_THRESHOLD_Y = -20.0f;       // Y position to trigger respawn
    constexpr float RESPAWN_HEIGHT = 5.0f;              // Height to respawn player at
    
    // ===== Player Physics =====
    constexpr float PLAYER_MOVE_SPEED = 5.0f;           // Horizontal movement speed (m/s)
    constexpr float PLAYER_JUMP_FORCE = 7.0f;           // Vertical jump impulse
    constexpr float PLAYER_MOUSE_SENSITIVITY = 0.002f;  // Mouse look sensitivity
    constexpr float PLAYER_CAMERA_Y_OFFSET = 0.7f;      // Camera height above player position
    
    // ===== Player Collider =====
    constexpr float PLAYER_COLLIDER_HEIGHT = 0.9f;      // Half-height of player capsule
    constexpr float PLAYER_COLLIDER_RADIUS = 0.4f;      // Radius of player capsule
    constexpr float PLAYER_COLLIDER_CENTER_Y = 0.9f;    // Y offset of collider center
    
    // ===== Gun Positioning (relative to camera) =====
    constexpr float GUN_OFFSET_RIGHT = 0.3f;            // Gun offset to the right
    constexpr float GUN_OFFSET_DOWN = -0.2f;            // Gun offset downward
    constexpr float GUN_OFFSET_FORWARD = 0.5f;          // Gun offset forward
    constexpr float GUN_SPAWN_OFFSET = 0.5f;            // Bullet spawn distance from gun
}
