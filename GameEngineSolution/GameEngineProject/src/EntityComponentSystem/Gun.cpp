#include "../../include/EntityComponentSystem/Gun.h"
#include "../../include/EntityComponentSystem/Bullet.h"
#include "../../include/Application/Scene.h"

Gun::Gun(Mesh* mesh, std::shared_ptr<Material> material)
    : GameObject(mesh, material), m_fireRate(0.2f), m_damage(10.0f), m_bulletSpeed(50.0f), m_lastShotTime(0.0f), m_owner(nullptr)
{
}

Gun::~Gun() = default;

void Gun::Update(float dt)
{
    if (m_lastShotTime > 0.0f)
    {
        m_lastShotTime -= dt;
    }
}

void Gun::Shoot(Scene* sceneInstance, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction)
{
    if (m_lastShotTime <= 0.0f)
    {
        // Spawn bullet via Scene
        if (sceneInstance)
        {
            sceneInstance->SpawnBullet(position, direction, m_bulletSpeed, m_damage);
            m_lastShotTime = m_fireRate;
        }
    }
}
