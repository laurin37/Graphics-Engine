#include "Graphics.h"
#include <vector>
#include <string>

// Link necessary libraries for DirectX
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// --- Embed Shaders ---
const char* vertexShaderSource = R"(
    cbuffer ConstantBuffer : register(b0)
    {
        matrix worldMatrix;
        matrix viewMatrix;
        matrix projectionMatrix;
    }

    struct VS_INPUT
    {
        float3 pos : POSITION;
        float2 uv : TEXCOORD;
    };

    struct PS_INPUT
    {
        float4 pos : SV_POSITION;
        float2 uv : TEXCOORD;
    };

    PS_INPUT main(VS_INPUT input)
    {
        PS_INPUT output;
        float4 pos = float4(input.pos, 1.0f);
        pos = mul(pos, worldMatrix);
        pos = mul(pos, viewMatrix);
        pos = mul(pos, projectionMatrix);
        output.pos = pos;
        output.uv = input.uv;
        return output;
    }
)";

const char* pixelShaderSource = R"(
    Texture2D proceduralTexture : register(t0);
    SamplerState pointSampler : register(s0);

    struct PS_INPUT
    {
        float4 pos : SV_POSITION;
        float2 uv : TEXCOORD;
    };

    float4 main(PS_INPUT input) : SV_TARGET
    {
        return proceduralTexture.Sample(pointSampler, input.uv);
    }
)";
// --- End Embed Shaders ---

Graphics::Graphics() : m_rotation(0.0f) {}

void Graphics::Initialize(HWND hwnd, int width, int height)
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;
    sd.OutputWindow = hwnd;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, new D3D_FEATURE_LEVEL{ D3D_FEATURE_LEVEL_11_0 }, 1,
        D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, nullptr, &m_deviceContext
    ));

    Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer;
    ThrowIfFailed(m_swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer));
    ThrowIfFailed(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView));

    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    ThrowIfFailed(m_device->CreateTexture2D(&depthStencilDesc, nullptr, &m_depthStencilBuffer));
    ThrowIfFailed(m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, &m_depthStencilView));

    m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<float>(width);
    vp.Height = static_cast<float>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    m_deviceContext->RSSetViewports(1, &vp);

    InitPipeline();

    m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV4,
        static_cast<float>(width) / static_cast<float>(height),
        0.1f, 100.0f
    );
}

void Graphics::InitPipeline()
{
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, pixelShaderBlob, errorBlob;
    
    HRESULT hr = D3DCompile(vertexShaderSource, strlen(vertexShaderSource), "VertexShader", nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertexShaderBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) { throw std::runtime_error(std::string("VS Error: ") + (char*)errorBlob->GetBufferPointer()); } else { ThrowIfFailed(hr); } }
    ThrowIfFailed(m_device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &m_vertexShader));

    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ThrowIfFailed(m_device->CreateInputLayout(inputLayoutDesc, ARRAYSIZE(inputLayoutDesc), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &m_inputLayout));

    hr = D3DCompile(pixelShaderSource, strlen(pixelShaderSource), "PixelShader", nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixelShaderBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) { throw std::runtime_error(std::string("PS Error: ") + (char*)errorBlob->GetBufferPointer()); } else { ThrowIfFailed(hr); } }
    ThrowIfFailed(m_device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &m_pixelShader));

    Vertex vertices[] =
    {
        // Front Face
        { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
        { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },
        { DirectX::XMFLOAT3( 0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
        { DirectX::XMFLOAT3( 0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
        // Back Face
        { DirectX::XMFLOAT3( 0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
        { DirectX::XMFLOAT3( 0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },
        { DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
        { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
        // Top Face
        { DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
        { DirectX::XMFLOAT3(-0.5f, 0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },
        { DirectX::XMFLOAT3( 0.5f, 0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
        { DirectX::XMFLOAT3( 0.5f, 0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
        // Bottom Face
        { DirectX::XMFLOAT3( 0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
        { DirectX::XMFLOAT3( 0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },
        { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
        { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
        // Left Face
        { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
        { DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },
        { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
        { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
        // Right Face
        { DirectX::XMFLOAT3( 0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
        { DirectX::XMFLOAT3( 0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },
        { DirectX::XMFLOAT3( 0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
        { DirectX::XMFLOAT3( 0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) }
    };
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA sd = {vertices, 0, 0};
    ThrowIfFailed(m_device->CreateBuffer(&bd, &sd, &m_vertexBuffer));

    unsigned int indices[] = {
        0, 1, 2,  0, 2, 3,    // Front
        4, 5, 6,  4, 6, 7,    // Back
        8, 9, 10, 8, 10, 11,  // Top
        12, 13, 14, 12, 14, 15, // Bottom
        16, 17, 18, 16, 18, 19, // Left
        20, 21, 22, 20, 22, 23  // Right
    };
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(unsigned int) * ARRAYSIZE(indices);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA isd = {indices, 0, 0};
    ThrowIfFailed(m_device->CreateBuffer(&ibd, &isd, &m_indexBuffer));

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(CB_VS_vertexshader);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ThrowIfFailed(m_device->CreateBuffer(&cbd, nullptr, &m_constantBuffer));

    // --- Procedural Texture ---
    const int texWidth = 64;
    const int texHeight = 64;
    std::vector<uint32_t> texData(texWidth * texHeight);
    for (int y = 0; y < texHeight; ++y) {
        for (int x = 0; x < texWidth; ++x) {
            texData[y * texWidth + x] = ((x / 8 % 2) == (y / 8 % 2)) ? 0xFFFFFFFF : 0xFF000000;
        }
    }
    
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = texWidth;
    texDesc.Height = texHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA texSubData = { texData.data(), texWidth * sizeof(uint32_t), 0 };
    ThrowIfFailed(m_device->CreateTexture2D(&texDesc, &texSubData, &texture));
    ThrowIfFailed(m_device->CreateShaderResourceView(texture.Get(), nullptr, &m_textureView));

    // --- Sampler State ---
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ThrowIfFailed(m_device->CreateSamplerState(&sampDesc, &m_samplerState));
}

void Graphics::RenderFrame()
{
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    m_rotation += 0.002f;
    if (m_rotation > 6.28f) m_rotation = 0.0f;

    m_worldMatrix = DirectX::XMMatrixRotationY(m_rotation) * DirectX::XMMatrixRotationX(m_rotation / 2.0f);
    m_viewMatrix = DirectX::XMMatrixLookAtLH({ 0.0f, 0.0f, -2.5f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });

    CB_VS_vertexshader cb;
    DirectX::XMStoreFloat4x4(&cb.worldMatrix, DirectX::XMMatrixTranspose(m_worldMatrix));
    DirectX::XMStoreFloat4x4(&cb.viewMatrix, DirectX::XMMatrixTranspose(m_viewMatrix));
    DirectX::XMStoreFloat4x4(&cb.projectionMatrix, DirectX::XMMatrixTranspose(m_projectionMatrix));

    m_deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
    m_deviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    m_deviceContext->PSSetShaderResources(0, 1, m_textureView.GetAddressOf());
    m_deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
    
    m_deviceContext->IASetInputLayout(m_inputLayout.Get());
    m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    m_deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_deviceContext->DrawIndexed(36, 0, 0);

    ThrowIfFailed(m_swapChain->Present(1, 0));
}
