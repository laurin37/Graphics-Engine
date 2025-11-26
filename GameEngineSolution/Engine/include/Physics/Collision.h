#pragma once

#include <DirectXMath.h>

struct AABB
{
    DirectX::XMFLOAT3 center;
    DirectX::XMFLOAT3 extents; // Half-width, half-height, half-depth
};

inline bool AABBIntersects(const AABB& a, const AABB& b)
{
    if (fabsf(a.center.x - b.center.x) > (a.extents.x + b.extents.x)) return false;
    if (fabsf(a.center.y - b.center.y) > (a.extents.y + b.extents.y)) return false;
    if (fabsf(a.center.z - b.center.z) > (a.extents.z + b.extents.z)) return false;
    return true;
}
