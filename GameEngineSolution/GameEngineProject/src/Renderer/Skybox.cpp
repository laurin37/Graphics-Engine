#include <vector>

#include "../../include/Utils/EnginePCH.h"
#include "../../include/Renderer/Skybox.h"
#include "../../include/Renderer/Mesh.h"
#include "../../include/ResourceManagement/Shader.h"
#include "../../include/EntityComponentSystem/Camera.h"
#include "../../include/ResourceManagement/TextureLoader.h"

// Constant buffer for skybox vertex shader
struct CB_VS_Skybox
{
    DirectX::XMMATRIX worldViewProj;
};

Skybox::Skybox() = default;
Skybox::~Skybox() = default;

void Skybox::Init(ID3D11Device* device, ID3D11DeviceContext* context, const std::wstring& textureFilename)
{
    // 1. Create Shaders
    D3D11_INPUT_ELEMENT_DESC skyboxLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    m_vs = std::make_unique<VertexShader>();
    m_vs->Init(device, L"Assets/Shaders/Skybox.hlsl", "VS_main", skyboxLayout, ARRAYSIZE(skyboxLayout));

    m_ps = std::make_unique<PixelShader>();
    m_ps->Init(device, L"Assets/Shaders/Skybox.hlsl", "PS_main");

    // 2. Create Cube Mesh
    std::vector<Vertex> vertices = {
        // Front face
        {DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.25f, 0.666f), {}, {}},
        {DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(0.25f, 0.333f), {}, {}},
        {DirectX::XMFLOAT3( 0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(0.5f, 0.333f), {}, {}},
        {DirectX::XMFLOAT3( 0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.5f, 0.666f), {}, {}},

        // Back face
        {DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), DirectX::XMFLOAT2(1.0f, 0.666f), {}, {}},
        {DirectX::XMFLOAT3( 0.5f, -0.5f, 0.5f), DirectX::XMFLOAT2(0.75f, 0.666f), {}, {}},
        {DirectX::XMFLOAT3( 0.5f,  0.5f, 0.5f), DirectX::XMFLOAT2(0.75f, 0.333f), {}, {}},
        {DirectX::XMFLOAT3(-0.5f,  0.5f, 0.5f), DirectX::XMFLOAT2(1.0f, 0.333f), {}, {}},
        
        // Top face
        {DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f), DirectX::XMFLOAT2(0.25f, 0.333f), {}, {}},
        {DirectX::XMFLOAT3(-0.5f, 0.5f,  0.5f), DirectX::XMFLOAT2(0.25f, 0.0f), {}, {}},
        {DirectX::XMFLOAT3( 0.5f, 0.5f,  0.5f), DirectX::XMFLOAT2(0.5f, 0.0f), {}, {}},
        {DirectX::XMFLOAT3( 0.5f, 0.5f, -0.5f), DirectX::XMFLOAT2(0.5f, 0.333f), {}, {}},

        // Bottom face
        {DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.25f, 1.0f), {}, {}},
        {DirectX::XMFLOAT3( 0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.5f, 1.0f), {}, {}},
        {DirectX::XMFLOAT3( 0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(0.5f, 0.666f), {}, {}},
        {DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(0.25f, 0.666f), {}, {}},

        // Left face
        {DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 0.666f), {}, {}},
        {DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 0.333f), {}, {}},
        {DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(0.25f, 0.333f), {}, {}},
        {DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.25f, 0.666f), {}, {}},

        // Right face
        {DirectX::XMFLOAT3( 0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.5f, 0.666f), {}, {}},
        {DirectX::XMFLOAT3( 0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(0.5f, 0.333f), {}, {}},
        {DirectX::XMFLOAT3( 0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(0.75f, 0.333f), {}, {}},
        {DirectX::XMFLOAT3( 0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(0.75f, 0.666f), {}, {}},
    };
    std::vector<unsigned int> indices = {
        0,1,2, 0,2,3,
        4,5,6, 4,6,7,
        8,9,10, 8,10,11,
        12,13,14, 12,14,15,
        16,17,18, 16,18,19,
        20,21,22, 20,22,23
    };
    m_mesh = std::make_unique<Mesh>(device, vertices, indices);

    // 3. Load Texture
    m_textureSRV = TextureLoader::Load(device, context, textureFilename);

    // 4. Create States
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    ThrowIfFailed(device->CreateDepthStencilState(&dsDesc, &m_dsState));

    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_FRONT;
    rsDesc.DepthClipEnable = TRUE;
    ThrowIfFailed(device->CreateRasterizerState(&rsDesc, &m_rsState));

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ThrowIfFailed(device->CreateSamplerState(&sampDesc, &m_samplerState));

    // 5. Create Constant Buffer
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(CB_VS_Skybox);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ThrowIfFailed(device->CreateBuffer(&cbd, nullptr, &m_constantBuffer));
}

void Skybox::Draw(ID3D11DeviceContext* context, const Camera& camera, const DirectX::XMMATRIX& projectionMatrix)
{
    context->RSSetState(m_rsState.Get());
    context->OMSetDepthStencilState(m_dsState.Get(), 0);

    // Get view matrix from camera but remove translation
    DirectX::XMMATRIX viewMatrix = camera.GetViewMatrix();
    viewMatrix.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixScaling(5.0f, 5.0f, 5.0f); // Scale up the skybox
    DirectX::XMMATRIX wvp = worldMatrix * viewMatrix * projectionMatrix;

    CB_VS_Skybox cb;
    cb.worldViewProj = DirectX::XMMatrixTranspose(wvp);
    context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    
    m_vs->Bind(context);
    m_ps->Bind(context);
    
    ID3D11ShaderResourceView* const srvs[] = { m_textureSRV.Get() };
    context->PSSetShaderResources(0, 1, srvs);
    context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    m_mesh->Draw(context);

    // Reset states
    context->RSSetState(nullptr);
    context->OMSetDepthStencilState(nullptr, 0);
}