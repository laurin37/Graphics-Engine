#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <DirectXMath.h>

// Vertex structure - moved here as it's fundamental to a mesh
struct Vertex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uv;
    DirectX::XMFLOAT3 normal;
};

class Mesh
{
public:
    Mesh(ID3D11Device* device, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh() = default;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void Draw(ID3D11DeviceContext* context) const;

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    UINT m_indexCount;
};
