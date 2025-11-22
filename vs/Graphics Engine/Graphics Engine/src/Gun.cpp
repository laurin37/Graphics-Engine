#include "include/Gun.h"
#include "include/Bullet.h"
#include "include/Game.h" // To add bullet to game world
#include <chrono>

Gun::Gun(Mesh* mesh, std::shared_ptr<Material> material)
    : GameObject(mesh, material), // Pass to base GameObject constructor
      m_fireRate(0.5f), // Shots per second
      m_damage(10.0f),
      m_bulletSpeed(50.0f),
      m_lastShotTime(0.0f),
      m_owner(nullptr)
{
    SetName(L"Gun"); // Set a default name for the Gun GameObject
}

Gun::~Gun()
{
}

void Gun::Update(float dt)
{
    // Advance last shot time
    // This allows fire rate to be controlled by the gun's own update
    m_lastShotTime += dt;
}

void Gun::Shoot(Game* gameInstance, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction)
{
    // Check if enough time has passed since the last shot
    if (m_lastShotTime < (1.0f / m_fireRate))
    {
        return; // Not ready to fire yet
    }

    // Reset shot timer
    m_lastShotTime = 0.0f;

    // Add the bullet to the game world
    if (gameInstance)
    {
        gameInstance->SpawnBullet(position, direction, m_bulletSpeed, m_damage);
    }
}
