#pragma once

#include "EnginePCH.h"
#include <DirectXMath.h>
#include "GameObject.h" // Gun will now be a GameObject

class Bullet; // Forward declaration
class Scene;   // Forward declaration

class Gun : public GameObject // Inherit from GameObject
{
public:
    Gun(Mesh* mesh, std::shared_ptr<Material> material); // New constructor
    ~Gun();

    void Update(float dt);
    void Shoot(Scene* sceneInstance, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction);

    // New method to set the owner
    void SetOwner(GameObject* owner) { m_owner = owner; }

    // Properties
    float m_fireRate;
    float m_damage;
    float m_bulletSpeed;
    float m_lastShotTime;

protected: // Changed to protected
    // Perhaps a reference or pointer to the GameObject this gun belongs to
    GameObject* m_owner; 
};
