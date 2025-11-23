#include "include/Player.h"
#include "include/Scene.h" 
#include "include/PhysicsSystem.h"
#include "include/Bullet.h"

using namespace DirectX;

Player::Player(Mesh* mesh, std::shared_ptr<Material> material, Camera* camera)
    : GameObject(mesh, material), m_camera(camera), m_velocity(0.0f, 0.0f, 0.0f), m_onGround(false)
{
    AABB playerBox;
    playerBox.center = { 0.0f, 0.9f, 0.0f };
    playerBox.extents = { 0.4f, 0.9f, 0.4f };
    SetBoundingBox(playerBox);
}

void Player::Update(float deltaTime, Input& input, const std::vector<std::unique_ptr<GameObject>>& worldObjects)
{
    // HYBRID APPROACH: Variable timestep with strict clamping for responsiveness + safety
    // Clamp deltaTime to prevent huge jumps during lag
    const float MIN_DT = 1.0f / 240.0f; // 4ms minimum
    const float MAX_DT = 1.0f / 30.0f;  // 33ms maximum (30 FPS floor)
    
    float dt = deltaTime;
    if (dt < MIN_DT) dt = MIN_DT;
    if (dt > MAX_DT) dt = MAX_DT; // If game drops below 30 FPS, slow down time instead of tunneling
    
    // Rotation
    float mouseSens = 0.002f;
    int dx = input.GetMouseDeltaX();
    int dy = input.GetMouseDeltaY();

    float rotY = GetRotation().y + dx * mouseSens;
    SetRotation(GetRotation().x, rotY, GetRotation().z);
    m_camera->AdjustRotation(dy * mouseSens, dx * mouseSens, 0.0f);

    DirectX::XMFLOAT3 pos = GetPosition();
    m_camera->SetPosition(pos.x, pos.y + 0.7f, pos.z);

    // Movement
    float yaw = GetRotation().y;
    DirectX::XMFLOAT3 forward = { sinf(yaw), 0.0f, cosf(yaw) };
    DirectX::XMFLOAT3 right = { cosf(yaw), 0.0f, -sinf(yaw) };

    DirectX::XMFLOAT3 moveDir = { 0.0f, 0.0f, 0.0f };
    if (input.IsKeyDown('W')) { moveDir.x += forward.x; moveDir.z += forward.z; }
    if (input.IsKeyDown('S')) { moveDir.x -= forward.x; moveDir.z -= forward.z; }
    if (input.IsKeyDown('A')) { moveDir.x -= right.x; moveDir.z -= right.z; }
    if (input.IsKeyDown('D')) { moveDir.x += right.x; moveDir.z += right.z; }

    float length = sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
    if (length > 0.0f) {
        moveDir.x /= length;
        moveDir.z /= length;
    }

    m_velocity.x = moveDir.x * MOVE_SPEED;
    m_velocity.z = moveDir.z * MOVE_SPEED;

    // Jumping
    if (m_onGround && input.IsKeyDown(VK_SPACE)) {
        m_velocity.y = JUMP_FORCE;
        m_onGround = false;
    }

    // Gravity with strict clamp
    m_velocity.y += GRAVITY * dt;
    const float MAX_FALL_SPEED = -15.0f;
    if (m_velocity.y < MAX_FALL_SPEED) {
        m_velocity.y = MAX_FALL_SPEED;
    }

    DirectX::XMFLOAT3 startPos = GetPosition();

    // Move X/Z
    SetPosition(startPos.x + m_velocity.x * dt, startPos.y, startPos.z + m_velocity.z * dt);
    
    for (const auto& obj : worldObjects) {
        if (obj.get() == this || obj.get() == m_gunPtr || dynamic_cast<Bullet*>(obj.get())) continue;
        if (PhysicsSystem::AABBIntersects(GetWorldBoundingBox(), obj->GetWorldBoundingBox())) {
            SetPosition(startPos.x, startPos.y, startPos.z);
            break;
        }
    }

    // Move Y (Predictive)
    startPos = GetPosition();
    if (startPos.y < -20.0f) {
        SetPosition(0.0f, 5.0f, 0.0f);
        m_velocity = { 0.0f, 0.0f, 0.0f };
        return;
    }

    float intendedY = startPos.y + m_velocity.y * dt;
    const float skinWidth = 0.005f;
    bool collisionDetected = false;

    AABB playerBox = GetWorldBoundingBox();
    AABB intendedBox = playerBox;
    float localOffset = playerBox.center.y - startPos.y;
    intendedBox.center.y = intendedY + localOffset;

    for (const auto& obj : worldObjects) {
        if (obj.get() == this || obj.get() == m_gunPtr || dynamic_cast<Bullet*>(obj.get())) continue;
        
        AABB objBox = obj->GetWorldBoundingBox();
        if (PhysicsSystem::AABBIntersects(intendedBox, objBox)) {
            collisionDetected = true;

            if (m_velocity.y < 0) {
                float objectTop = objBox.center.y + objBox.extents.y;
                float resolvedY = objectTop - localOffset + playerBox.extents.y;
                SetPosition(startPos.x, resolvedY, startPos.z);
                m_velocity.y = 0;
            }
            else if (m_velocity.y > 0) {
                float objectBottom = objBox.center.y - objBox.extents.y;
                float resolvedY = objectBottom - localOffset - playerBox.extents.y;
                SetPosition(startPos.x, resolvedY, startPos.z);
                m_velocity.y = 0;
            }
            break;
        }
    }

    if (!collisionDetected) {
        SetPosition(startPos.x, intendedY, startPos.z);
    }

    // Ground Check
    m_onGround = false;
    AABB footProbe = GetWorldBoundingBox();
    footProbe.center.y -= (skinWidth * 3.0f);

    for (const auto& obj : worldObjects) {
        if (obj.get() == this || obj.get() == m_gunPtr || dynamic_cast<Bullet*>(obj.get())) continue;
        
        AABB objBox = obj->GetWorldBoundingBox();
        if (PhysicsSystem::AABBIntersects(footProbe, objBox)) {
            m_onGround = true;
            
            AABB currentBox = GetWorldBoundingBox();
            if (PhysicsSystem::AABBIntersects(currentBox, objBox) && m_velocity.y <= 0) {
                float objectTop = objBox.center.y + objBox.extents.y;
                float currentBottom = currentBox.center.y - currentBox.extents.y;
                
                if (currentBottom < objectTop) {
                    float localCenterY = currentBox.center.y - GetPosition().y;
                    float correctedY = objectTop - localCenterY + currentBox.extents.y;
                    SetPosition(GetPosition().x, correctedY, GetPosition().z);
                    m_velocity.y = 0;
                }
            }
            break;
        }
    }

    // Update Gun
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

        m_gunPtr->Update(dt);
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

//  Unused helper methods from fixed timestep attempt (kept for reference)
void Player::UpdatePhysicsStep(float dt, Input& input, const std::vector<std::unique_ptr<GameObject>>& worldObjects) {}
void Player::UpdateCameraAndGun(Input& input, float deltaTime) {}