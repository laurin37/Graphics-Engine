#pragma once

#include "EnginePCH.h"
#include <DirectXMath.h>
#include "GameObject.h"
#include "Mesh.h"       // Include actual Mesh header
#include "Material.h"   // Include actual Material header

class Bullet : public GameObject
{
public:
    Bullet();
    Bullet(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction, float speed, float damage);
    ~Bullet();

    void Update(float dt);

    void SetDirection(const DirectX::XMFLOAT3& direction);
    DirectX::XMFLOAT3 GetDirection() const { return m_direction; }

    float GetDamage() const { return m_damage; }

    // New methods for lifecycle management
    float GetAge() const { return m_age; }
    void SetActive(bool active) { m_active = active; }
    bool IsActive() const { return m_active; }

private:
    DirectX::XMFLOAT3 m_direction;
    float m_speed;
    float m_damage;
    bool m_active; // To manage its lifecycle
    float m_age; // How long the bullet has been alive
};