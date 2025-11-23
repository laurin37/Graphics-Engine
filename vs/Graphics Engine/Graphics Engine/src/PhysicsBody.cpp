#include "include/PhysicsBody.h"
#include "include/GameObject.h"
#include "include/PhysicsSystem.h"
#include "include/Bullet.h"
#include "include/PhysicsConstants.h"

using namespace DirectX;
using namespace PhysicsConstants;

PhysicsBody::PhysicsBody()
    : velocity(0.0f, 0.0f, 0.0f), 
      m_accumulatedForces(0.0f, 0.0f, 0.0f),
      isGrounded(false)
{
}

void PhysicsBody::SetVelocity(const DirectX::XMFLOAT3& vel)
{
    velocity = vel;
}

void PhysicsBody::SetVelocity(float x, float y, float z)
{
    velocity = { x, y, z };
}

void PhysicsBody::AddForce(const DirectX::XMFLOAT3& force)
{
    if (!isKinematic) {
        m_accumulatedForces.x += force.x;
        m_accumulatedForces.y += force.y;
        m_accumulatedForces.z += force.z;
    }
}

void PhysicsBody::AddImpulse(const DirectX::XMFLOAT3& impulse)
{
    if (!isKinematic) {
        velocity.x += impulse.x;
        velocity.y += impulse.y;
        velocity.z += impulse.z;
    }
}

void PhysicsBody::Update(float dt, GameObject* owner,
                         const std::vector<std::unique_ptr<GameObject>>& worldObjects,
                         const std::vector<GameObject*>& ignoreList)
{
    if (isKinematic) {
        // Kinematic objects don't simulate physics
        return;
    }
    
    // Apply accumulated forces (F = ma, so a = F/m)
    velocity.x += (m_accumulatedForces.x / mass) * dt;
    velocity.y += (m_accumulatedForces.y / mass) * dt;
    velocity.z += (m_accumulatedForces.z / mass) * dt;
    m_accumulatedForces = { 0.0f, 0.0f, 0.0f }; // Clear for next frame
    
    // Apply gravity
    if (useGravity) {
        ApplyGravity(dt);
    }
    
    // Apply drag
    if (drag > 0.0f) {
        ApplyDrag(dt);
    }
    
    // Clamp velocity
    ClampVelocity();
    
    // Integrate velocity into position
    if (checkCollisions) {
        ResolveCollisions(dt, owner, worldObjects, ignoreList);
    } else {
        IntegrateVelocity(dt, owner);
    }
    
    // Check if grounded
    CheckGroundState(owner, worldObjects, ignoreList);
}

void PhysicsBody::ApplyGravity(float dt)
{
    velocity.y += gravityAcceleration * dt;
}

void PhysicsBody::ApplyDrag(float dt)
{
    float dragFactor = 1.0f - (drag * dt);
    if (dragFactor < 0.0f) dragFactor = 0.0f;
    
    velocity.x *= dragFactor;
    velocity.z *= dragFactor;
}

void PhysicsBody::ClampVelocity()
{
    if (velocity.y < maxFallSpeed) {
        velocity.y = maxFallSpeed;
    }
}

void PhysicsBody::IntegrateVelocity(float dt, GameObject* owner)
{
    DirectX::XMFLOAT3 pos = owner->GetPosition();
    pos.x += velocity.x * dt;
    pos.y += velocity.y * dt;
    pos.z += velocity.z * dt;
    owner->SetPosition(pos.x, pos.y, pos.z);
}

void PhysicsBody::ResolveCollisions(float dt, GameObject* owner,
                                    const std::vector<std::unique_ptr<GameObject>>& worldObjects,
                                    const std::vector<GameObject*>& ignoreList)
{
    // Clamp deltaTime for safety to prevent physics explosions
    if (dt < MIN_DELTA_TIME) dt = MIN_DELTA_TIME;
    if (dt > MAX_DELTA_TIME) dt = MAX_DELTA_TIME;
    
    DirectX::XMFLOAT3 startPos = owner->GetPosition();
    
    // === Move X/Z (horizontal) ===
    owner->SetPosition(startPos.x + velocity.x * dt, startPos.y, startPos.z + velocity.z * dt);
    
    for (const auto& obj : worldObjects) {
        // Check ignore list
        bool shouldIgnore = false;
        for (const auto& ignoreObj : ignoreList) {
            if (obj.get() == ignoreObj) {
                shouldIgnore = true;
                break;
            }
        }
        if (shouldIgnore || obj.get() == owner) continue;
        
        AABB ownerBox = owner->GetWorldBoundingBox();
        AABB objBox = obj->GetWorldBoundingBox();
        
        // Check for horizontal overlap
        bool xOverlap = fabsf(ownerBox.center.x - objBox.center.x) <= (ownerBox.extents.x + objBox.extents.x);
        bool zOverlap = fabsf(ownerBox.center.z - objBox.center.z) <= (ownerBox.extents.z + objBox.extents.z);
        
        if (xOverlap && zOverlap) {
            // There's horizontal overlap - but is it a wall or are we standing on top?
            float ownerBottom = ownerBox.center.y - ownerBox.extents.y;
            float objTop = objBox.center.y + objBox.extents.y;
            
            // If we're standing on top of this object, allow horizontal movement
            // (player bottom is at or slightly above object top)
            if (ownerBottom >= objTop - STANDING_TOLERANCE) {
                // Standing on top - allow movement
                continue;
            }
            
            // Otherwise, it's a wall collision - block movement
            owner->SetPosition(startPos.x, startPos.y, startPos.z);
            velocity.x = 0;
            velocity.z = 0;
            break;
        }
    }
    
    // === Move Y (vertical with predictive collision) ===
    startPos = owner->GetPosition();
    
    // Safety: respawn if fallen too far
    if (startPos.y < RESPAWN_THRESHOLD_Y) {
        owner->SetPosition(0.0f, RESPAWN_HEIGHT, 0.0f);
        velocity = { 0.0f, 0.0f, 0.0f };
        return;
    }
    
    float intendedY = startPos.y + velocity.y * dt;
    bool collisionDetected = false;
    
    AABB ownerBox = owner->GetWorldBoundingBox();
    AABB intendedBox = ownerBox;
    float localOffset = ownerBox.center.y - startPos.y;
    intendedBox.center.y = intendedY + localOffset;
    
    for (const auto& obj : worldObjects) {
        // Check ignore list
        bool shouldIgnore = false;
        for (const auto& ignoreObj : ignoreList) {
            if (obj.get() == ignoreObj) {
                shouldIgnore = true;
                break;
            }
        }
        if (shouldIgnore || obj.get() == owner) continue;
        
        AABB objBox = obj->GetWorldBoundingBox();
        if (PhysicsSystem::AABBIntersects(intendedBox, objBox)) {
            collisionDetected = true;
            
            if (velocity.y < 0) {
                // Landing
                float objectTop = objBox.center.y + objBox.extents.y;
                float resolvedY = objectTop - localOffset + ownerBox.extents.y;
                owner->SetPosition(startPos.x, resolvedY, startPos.z);
                velocity.y = 0;
            }
            else if (velocity.y > 0) {
                // Head bump
                float objectBottom = objBox.center.y - objBox.extents.y;
                float resolvedY = objectBottom - localOffset - ownerBox.extents.y;
                owner->SetPosition(startPos.x, resolvedY, startPos.z);
                velocity.y = 0;
            }
            break;
        }
    }
    
    if (!collisionDetected) {
        owner->SetPosition(startPos.x, intendedY, startPos.z);
    }
}

void PhysicsBody::CheckGroundState(GameObject* owner,
                                   const std::vector<std::unique_ptr<GameObject>>& worldObjects,
                                   const std::vector<GameObject*>& ignoreList)
{
    isGrounded = false;
    
    // Probe slightly below player to check for ground
    AABB footProbe = owner->GetWorldBoundingBox();
    footProbe.center.y -= GROUND_PROBE_DISTANCE;
    
    for (const auto& obj : worldObjects) {
        // Check ignore list
        bool shouldIgnore = false;
        for (const auto& ignoreObj : ignoreList) {
            if (obj.get() == ignoreObj) {
                shouldIgnore = true;
                break;
            }
        }
        if (shouldIgnore || obj.get() == owner) continue;
        
        AABB objBox = obj->GetWorldBoundingBox();
        if (PhysicsSystem::AABBIntersects(footProbe, objBox)) {
            isGrounded = true;
            
            // Emergency penetration recovery
            AABB currentBox = owner->GetWorldBoundingBox();
            if (PhysicsSystem::AABBIntersects(currentBox, objBox) && velocity.y <= 0) {
                float objectTop = objBox.center.y + objBox.extents.y;
                float currentBottom = currentBox.center.y - currentBox.extents.y;
                
                if (currentBottom < objectTop) {
                    float localCenterY = currentBox.center.y - owner->GetPosition().y;
                    float correctedY = objectTop - localCenterY + currentBox.extents.y;
                    owner->SetPosition(owner->GetPosition().x, correctedY, owner->GetPosition().z);
                    velocity.y = 0;
                }
            }
            break;
        }
    }
}
