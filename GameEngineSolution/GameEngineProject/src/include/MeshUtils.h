#pragma once
#include "Mesh.h"
#include "Collision.h"
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>

// Undefine Windows macros that conflict with std::min/max
#undef min
#undef max

namespace MeshUtils
{
    // Calculate tight-fitting AABB from mesh vertex data
    inline AABB CalculateAABB(const Mesh* mesh)
    {
        if (!mesh || mesh->GetVertexCount() == 0) {
            // Return default empty AABB
            return AABB{ {0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f} };
        }

        const std::vector<Vertex>& vertices = mesh->GetVertices();
        size_t vertexCount = vertices.size();

        // Initialize with first vertex
        DirectX::XMFLOAT3 minBounds = vertices[0].pos;
        DirectX::XMFLOAT3 maxBounds = vertices[0].pos;

        // Find min/max bounds
        for (size_t i = 1; i < vertexCount; ++i) {
            const DirectX::XMFLOAT3& pos = vertices[i].pos;
            
            minBounds.x = (std::min)(minBounds.x, pos.x);
            minBounds.y = (std::min)(minBounds.y, pos.y);
            minBounds.z = (std::min)(minBounds.z, pos.z);
            
            maxBounds.x = (std::max)(maxBounds.x, pos.x);
            maxBounds.y = (std::max)(maxBounds.y, pos.y);
            maxBounds.z = (std::max)(maxBounds.z, pos.z);
        }

        // Calculate center and extents (half-sizes)
        AABB result;
        result.center.x = (minBounds.x + maxBounds.x) * 0.5f;
        result.center.y = (minBounds.y + maxBounds.y) * 0.5f;
        result.center.z = (minBounds.z + maxBounds.z) * 0.5f;

        result.extents.x = (maxBounds.x - minBounds.x) * 0.5f;
        result.extents.y = (maxBounds.y - minBounds.y) * 0.5f;
        result.extents.z = (maxBounds.z - minBounds.z) * 0.5f;

        return result;
    }

    // Bounding sphere structure
    struct Sphere {
        DirectX::XMFLOAT3 center;
        float radius;
    };

    // Calculate bounding sphere from mesh (simple center + max distance)
    inline Sphere CalculateBoundingSphere(const Mesh* mesh)
    {
        AABB aabb = CalculateAABB(mesh);
        
        Sphere sphere;
        sphere.center = aabb.center;
        
        // Radius = distance from center to corner of AABB
        sphere.radius = sqrtf(
            aabb.extents.x * aabb.extents.x +
            aabb.extents.y * aabb.extents.y +
            aabb.extents.z * aabb.extents.z
        );
        
        return sphere;
    }
}
