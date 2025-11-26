#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <DirectXMath.h>

#include "../Utils/EnginePCH.h"
#include "../Physics/Collision.h"

// Vertex structure - this is the input for our meshes
struct Vertex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uv;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT3 tangent;
};

class Mesh
{
public:
    Mesh(ID3D11Device* device, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh() = default;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void Draw(ID3D11DeviceContext* context) const;
    
    // Accessors for collision generation
    const std::vector<Vertex>& GetVertices() const { return m_vertices; }
    size_t GetVertexCount() const { return m_vertices.size(); }
    const AABB& GetLocalBounds() const { return m_bounds; }

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    UINT m_indexCount;
    
    // Store vertex data for collision generation
    std::vector<Vertex> m_vertices;
    AABB m_bounds{};
};
