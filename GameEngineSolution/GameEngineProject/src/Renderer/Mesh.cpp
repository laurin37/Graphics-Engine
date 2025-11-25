#include "../../include/Renderer/Mesh.h"
#include <algorithm>
#include <cfloat>

Mesh::Mesh(ID3D11Device* device, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : m_vertices(vertices) // Store for collision generation
{
    m_indexCount = static_cast<UINT>(indices.size());

    // Create Vertex Buffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA sd = { vertices.data(), 0, 0 };
    ThrowIfFailed(device->CreateBuffer(&bd, &sd, &m_vertexBuffer));

    // Create Index Buffer
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = static_cast<UINT>(sizeof(unsigned int) * indices.size());
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA isd = { indices.data(), 0, 0 };
    ThrowIfFailed(device->CreateBuffer(&ibd, &isd, &m_indexBuffer));

    DirectX::XMFLOAT3 minPos = { FLT_MAX, FLT_MAX, FLT_MAX };
    DirectX::XMFLOAT3 maxPos = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
    for (const auto& vertex : m_vertices)
    {
        minPos.x = (std::min)(minPos.x, vertex.pos.x);
        minPos.y = (std::min)(minPos.y, vertex.pos.y);
        minPos.z = (std::min)(minPos.z, vertex.pos.z);

        maxPos.x = (std::max)(maxPos.x, vertex.pos.x);
        maxPos.y = (std::max)(maxPos.y, vertex.pos.y);
        maxPos.z = (std::max)(maxPos.z, vertex.pos.z);
    }

    m_bounds.center = {
        (minPos.x + maxPos.x) * 0.5f,
        (minPos.y + maxPos.y) * 0.5f,
        (minPos.z + maxPos.z) * 0.5f
    };

    m_bounds.extents = {
        (maxPos.x - minPos.x) * 0.5f,
        (maxPos.y - minPos.y) * 0.5f,
        (maxPos.z - minPos.z) * 0.5f
    };
}

void Mesh::Draw(ID3D11DeviceContext* context) const
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(m_indexCount, 0, 0);
}
