#include "../../include/EntityComponentSystem/Bullet.h"

using namespace DirectX;

Bullet::Bullet()
    : GameObject(nullptr, std::shared_ptr<Material>()), // Pass default values to base constructor
      m_direction(0.0f, 0.0f, 0.0f),
      m_speed(0.0f),
      m_damage(0.0f),
      m_active(false),
      m_age(0.0f) // Initialize age
{
    SetName(L"Bullet"); // Use SetName()
}

Bullet::Bullet(const XMFLOAT3& position, const XMFLOAT3& direction, float speed, float damage)
    : GameObject(nullptr, std::shared_ptr<Material>()), // Pass default values to base constructor
      m_direction(direction),
      m_speed(speed),
      m_damage(damage),
      m_active(true),
      m_age(0.0f) // Initialize age
{
    m_transform.SetPosition(position.x, position.y, position.z);
    SetName(L"Bullet"); // Use SetName()
}

Bullet::~Bullet()
{
}

void Bullet::Update(float dt)
{
    if (!m_active) return;

    m_age += dt; // Update age

    // Calculate new position based on direction and speed (no gravity)
    XMFLOAT3 currentPosFloat3 = m_transform.GetPosition(); // Store in a temporary variable
    XMVECTOR currentPos = XMLoadFloat3(&currentPosFloat3);
    XMVECTOR dir = XMLoadFloat3(&m_direction);
    XMVECTOR newPos = XMVectorAdd(currentPos, XMVectorScale(dir, m_speed * dt));

    XMFLOAT3 newPosF3;
    XMStoreFloat3(&newPosF3, newPos);
    m_transform.SetPosition(newPosF3.x, newPosF3.y, newPosF3.z); // Pass individual components
}

void Bullet::SetDirection(const XMFLOAT3& direction)
{
    m_direction = direction;
    XMVECTOR dirVec = XMLoadFloat3(&m_direction);
    dirVec = XMVector3Normalize(dirVec);
    XMStoreFloat3(&m_direction, dirVec);
}
