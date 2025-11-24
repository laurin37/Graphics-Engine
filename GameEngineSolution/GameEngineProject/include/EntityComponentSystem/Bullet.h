#pragma once

#include <DirectXMath.h>

#include "../Utils/EnginePCH.h"
#include "GameObject.h"
#include "../Renderer/Mesh.h"
#include "../Renderer/Material.h"

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