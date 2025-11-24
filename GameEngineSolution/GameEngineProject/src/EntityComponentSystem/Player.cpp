#include "../../include/EntityComponentSystem/Player.h"
#include "../../include/Application/Scene.h" 
#include "../../include/Physics/PhysicsSystem.h"
#include "../../include/EntityComponentSystem/Bullet.h"

using namespace DirectX;

Player::Player(Mesh* mesh, std::shared_ptr<Material> material, Camera* camera)
    : GameObject(mesh, material), m_camera(camera)
{
    // Setup bounding box
    AABB playerBox;
    playerBox.center = { 0.0f, 0.9f, 0.0f };
    playerBox.extents = { 0.4f, 0.9f, 0.4f };
    SetBoundingBox(playerBox);
    
    // Configure physics
    m_physicsBody.useGravity = true;
    m_physicsBody.checkCollisions = true;
    m_physicsBody.gravityAcceleration = -15.0f;
    m_physicsBody.maxFallSpeed = -15.0f;
    
    // Link physics to this GameObject
    SetPhysics(&m_physicsBody);
}

void Player::Update(float deltaTime, Input& input, const std::vector<std::unique_ptr<GameObject>>& worldObjects)
{
    // Handle input and apply forces
    HandleInput(input, deltaTime);
    
    // Run physics simulation
    std::vector<GameObject*> ignoreList = { m_gunPtr };
    m_physicsBody.Update(deltaTime, this, worldObjects, ignoreList);
    
    // Update camera and gun based on final position
    UpdateCameraAndGun(deltaTime);
}

void Player::HandleInput(Input& input, float deltaTime)
{
    // Mouse look (rotation)
    float mouseSens = 0.001f;
    int dx = input.GetMouseDeltaX();
    int dy = input.GetMouseDeltaY();

    float rotY = GetRotation().y + dx * mouseSens;
    SetRotation(GetRotation().x, rotY, GetRotation().z);
    m_camera->AdjustRotation(dy * mouseSens, dx * mouseSens, 0.0f);

    // WASD movement
    float yaw = GetRotation().y;
    DirectX::XMFLOAT3 forward = { sinf(yaw), 0.0f, cosf(yaw) };
    DirectX::XMFLOAT3 right = { cosf(yaw), 0.0f, -sinf(yaw) };

    DirectX::XMFLOAT3 moveDir = { 0.0f, 0.0f, 0.0f };
    if (input.IsKeyDown('W')) { moveDir.x += forward.x; moveDir.z += forward.z; }
    if (input.IsKeyDown('S')) { moveDir.x -= forward.x; moveDir.z -= forward.z; }
    if (input.IsKeyDown('A')) { moveDir.x -= right.x; moveDir.z -= right.z; }
    if (input.IsKeyDown('D')) { moveDir.x += right.x; moveDir.z += right.z; }

    // Normalize
    float length = sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
    if (length > 0.0f) {
        moveDir.x /= length;
        moveDir.z /= length;
    }

    // Set horizontal velocity directly (instant response)
    m_physicsBody.velocity.x = moveDir.x * MOVE_SPEED;
    m_physicsBody.velocity.z = moveDir.z * MOVE_SPEED;

    // Jumping
    if (m_physicsBody.isGrounded && input.IsKeyDown(VK_SPACE)) {
        m_physicsBody.AddImpulse({ 0.0f, JUMP_FORCE, 0.0f });
    }
}

void Player::UpdateCameraAndGun(float deltaTime)
{
    // Update camera position
    DirectX::XMFLOAT3 pos = GetPosition();
    m_camera->SetPosition(pos.x, pos.y + 0.7f, pos.z);

    // Update gun position and rotation
    if (m_gunPtr) {
        DirectX::XMFLOAT3 camPos = m_camera->GetPositionFloat3();
        DirectX::XMVECTOR camForwardVec = m_camera->GetForward();
        DirectX::XMVECTOR camRightVec = m_camera->GetRight();
        DirectX::XMVECTOR camUpVec = m_camera->GetUp();

        DirectX::XMFLOAT3 camForward, camRight, camUp;
        XMStoreFloat3(&camForward, camForwardVec);
        XMStoreFloat3(&camRight, camRightVec);
        XMStoreFloat3(&camUp, camUpVec);

        DirectX::XMFLOAT3 gunPos;
        gunPos.x = camPos.x + camRight.x * 0.3f - camUp.x * 0.2f + camForward.x * 0.5f;
        gunPos.y = camPos.y + camRight.y * 0.3f - camUp.y * 0.2f + camForward.y * 0.5f;
        gunPos.z = camPos.z + camRight.z * 0.3f - camUp.z * 0.2f + camForward.z * 0.5f;

        m_gunPtr->SetPosition(gunPos.x, gunPos.y, gunPos.z);
        
        DirectX::XMVECTOR camRotVec = m_camera->GetRotation();
        DirectX::XMFLOAT3 camRot;
        XMStoreFloat3(&camRot, camRotVec);
        m_gunPtr->SetRotation(camRot.x, camRot.y, camRot.z);

        m_gunPtr->Update(deltaTime);
    }
}

void Player::Shoot(Scene* sceneInstance)
{
    if (m_gunPtr) {
        DirectX::XMVECTOR shootDirVec = m_camera->GetForward();
        DirectX::XMFLOAT3 shootDir;
        XMStoreFloat3(&shootDir, shootDirVec);

        DirectX::XMFLOAT3 shootPos = m_gunPtr->GetPosition();
        shootPos.x += shootDir.x * 0.5f;
        shootPos.y += shootDir.y * 0.5f;
        shootPos.z += shootDir.z * 0.5f;

        m_gunPtr->Shoot(sceneInstance, shootPos, shootDir);
    }
}