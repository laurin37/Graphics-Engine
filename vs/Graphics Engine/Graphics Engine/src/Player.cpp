#include "include/Player.h"
#include "include/Scene.h" 
#include "include/PhysicsSystem.h"
#include "include/Bullet.h"

using namespace DirectX;

Player::Player(Mesh* mesh, std::shared_ptr<Material> material, Camera* camera)
    : GameObject(mesh, material), m_camera(camera), m_velocity(0.0f, 0.0f, 0.0f), m_onGround(false)
{
    // Set a smaller bounding box for the player (human size)
    AABB playerBox;
    playerBox.center = { 0, 1.0f, 0 };
    playerBox.extents = { 0.4f, 0.9f, 0.4f };
    SetBoundingBox(playerBox);
}

void Player::Update(float deltaTime, Input& input, const std::vector<std::unique_ptr<GameObject>>& worldObjects)
{
    // --- Rotation (Mouse Look) ---
    float mouseSens = 0.002f;
    int dx = input.GetMouseDeltaX();
    int dy = input.GetMouseDeltaY();

    // Rotate Player (Y-axis)
    float rotY = GetRotation().y + dx * mouseSens;
    SetRotation(GetRotation().x, rotY, GetRotation().z);

    // Rotate Camera (X-axis, clamped)
    // Note: Camera rotation is handled by Camera class, but we drive it here
    m_camera->AdjustRotation(dy * mouseSens, dx * mouseSens, 0.0f);

    // Sync Camera Position to Player (First Person)
    DirectX::XMFLOAT3 pos = GetPosition();
    m_camera->SetPosition(pos.x, pos.y + 0.8f, pos.z); // Eye level

    // --- Movement (WASD) ---
    // Calculate Forward and Right vectors based on Player's Y rotation
    float yaw = GetRotation().y;
    DirectX::XMFLOAT3 forward = { sinf(yaw), 0.0f, cosf(yaw) };
    DirectX::XMFLOAT3 right = { cosf(yaw), 0.0f, -sinf(yaw) };

    DirectX::XMFLOAT3 moveDir = { 0.0f, 0.0f, 0.0f };

    if (input.IsKeyDown('W'))
    {
        moveDir.x += forward.x;
        moveDir.z += forward.z;
    }
    if (input.IsKeyDown('S'))
    {
        moveDir.x -= forward.x;
        moveDir.z -= forward.z;
    }
    if (input.IsKeyDown('A'))
    {
        moveDir.x -= right.x;
        moveDir.z -= right.z;
    }
    if (input.IsKeyDown('D'))
    {
        moveDir.x += right.x;
        moveDir.z += right.z;
    }

    // Normalize moveDir
    float length = sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
    if (length > 0.0f)
    {
        moveDir.x /= length;
        moveDir.z /= length;
    }

    // Apply Movement
    // Update velocity based on input (instant acceleration for responsiveness)
    m_velocity.x = moveDir.x * MOVE_SPEED;
    m_velocity.z = moveDir.z * MOVE_SPEED;

    // --- Jumping ---
    if (m_onGround && input.IsKeyDown(VK_SPACE))
    {
        m_velocity.y = JUMP_FORCE;
        m_onGround = false;
    }

    // --- Gravity ---
    m_velocity.y += GRAVITY * deltaTime;

    // --- Collision Detection & Resolution (Simple AABB) ---
    // We move X/Z first, check collision, then Y, check collision.
    
    DirectX::XMFLOAT3 startPos = GetPosition();

    // --- Move X/Z ---
    SetPosition(startPos.x + m_velocity.x * deltaTime, startPos.y, startPos.z + m_velocity.z * deltaTime);
    
    // Check Wall Collisions
    for (const auto& obj : worldObjects)
    {
        if (obj.get() == this) continue; // Skip self
        if (obj.get() == m_gunPtr) continue; // Skip own gun
        if (dynamic_cast<Bullet*>(obj.get())) continue; // Skip bullets
        
        if (PhysicsSystem::AABBIntersects(GetWorldBoundingBox(), obj->GetWorldBoundingBox()))
        {
            // Revert X/Z movement on collision
            SetPosition(startPos.x, startPos.y, startPos.z);
            break;
        }
    }

    // --- Move Y ---
    startPos = GetPosition();
    SetPosition(startPos.x, startPos.y + m_velocity.y * deltaTime, startPos.z);
    m_onGround = false; // Assume in air until collision
    
    // Safety check: if we fell too far, reset (simple respawn logic)
    if (GetPosition().y < -20.0f) {
        SetPosition(0.0f, 5.0f, 0.0f);
        m_velocity = { 0.0f, 0.0f, 0.0f };
    }

    for (const auto& obj : worldObjects) {
        if (obj.get() == this) continue;
        if (obj.get() == m_gunPtr) continue; // Skip own gun
        if (dynamic_cast<Bullet*>(obj.get())) continue; // Skip bullets
        
        AABB playerBox = GetWorldBoundingBox();
        AABB objBox = obj->GetWorldBoundingBox();

        if (PhysicsSystem::AABBIntersects(playerBox, objBox)) {
            // If falling down and hit something, we are on ground
            if (m_velocity.y < 0) {
                m_onGround = true;
                m_velocity.y = 0; // Stop gravity accumulation
                
                // Snap to top of object to prevent sinking
                // The player's AABB center is offset from the transform position
                // We need to account for this offset when positioning
                float objectTop = objBox.center.y + objBox.extents.y;
                float playerHalfHeight = playerBox.extents.y;
                
                // Calculate the offset between transform position and AABB center
                float aabbCenterOffset = playerBox.center.y - startPos.y;
                
                // New position: object top + player half height - AABB offset
                float newY = objectTop + playerHalfHeight - aabbCenterOffset + 0.001f;
                SetPosition(startPos.x, newY, startPos.z);
            }
            else if (m_velocity.y > 0) {
                // Hit something above (head bump)
                m_velocity.y = 0;
                SetPosition(startPos.x, startPos.y, startPos.z); // Revert Y
            }
            break;
        }
    }

    // --- Update Gun Position ---
    if (m_gunPtr)
    {
        // Calculate Gun Position relative to Camera
        // We want the gun to be slightly to the right and down from the camera/eye position
        // Camera Position is at Eye Level (set above)
        
        DirectX::XMFLOAT3 camPos = m_camera->GetPositionFloat3(); // Use GetPositionFloat3
        DirectX::XMVECTOR camForwardVec = m_camera->GetForward(); // Use GetForward
        DirectX::XMVECTOR camRightVec = m_camera->GetRight();     // Use GetRight
        DirectX::XMVECTOR camUpVec = m_camera->GetUp();           // Use GetUp

        DirectX::XMFLOAT3 camForward, camRight, camUp;
        XMStoreFloat3(&camForward, camForwardVec);
        XMStoreFloat3(&camRight, camRightVec);
        XMStoreFloat3(&camUp, camUpVec);

        // Offset: Right 0.3, Down 0.2, Forward 0.5
        DirectX::XMFLOAT3 gunPos;
        gunPos.x = camPos.x + camRight.x * 0.3f - camUp.x * 0.2f + camForward.x * 0.5f;
        gunPos.y = camPos.y + camRight.y * 0.3f - camUp.y * 0.2f + camForward.y * 0.5f;
        gunPos.z = camPos.z + camRight.z * 0.3f - camUp.z * 0.2f + camForward.z * 0.5f;

        m_gunPtr->SetPosition(gunPos.x, gunPos.y, gunPos.z);
        
        // Match Camera Rotation
        DirectX::XMVECTOR camRotVec = m_camera->GetRotation();
        DirectX::XMFLOAT3 camRot;
        XMStoreFloat3(&camRot, camRotVec);
        m_gunPtr->SetRotation(camRot.x, camRot.y, camRot.z);

        m_gunPtr->Update(deltaTime); // Update Gun (timers, etc.)
    }
}

void Player::Shoot(Scene* sceneInstance)
{
    if (m_gunPtr)
    {
        // Use the camera's forward vector for shooting direction, not the player's body forward
        // This ensures bullets go where the player is looking (crosshair)
        DirectX::XMVECTOR shootDirVec = m_camera->GetForward();
        DirectX::XMFLOAT3 shootDir;
        XMStoreFloat3(&shootDir, shootDirVec);

        DirectX::XMFLOAT3 shootPos = m_gunPtr->GetPosition(); // Spawn from gun

        // Adjust spawn position to be slightly in front of the gun to avoid self-collision
        shootPos.x += shootDir.x * 0.5f;
        shootPos.y += shootDir.y * 0.5f;
        shootPos.z += shootDir.z * 0.5f;

        m_gunPtr->Shoot(sceneInstance, shootPos, shootDir);
    }
}