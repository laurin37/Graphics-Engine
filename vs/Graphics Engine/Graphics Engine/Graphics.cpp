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
        float4 color : COLOR;
    };

    struct PS_INPUT
    {
        float4 pos : SV_POSITION;
        float4 color : COLOR;
    };

    PS_INPUT main(VS_INPUT input)
    {
        PS_INPUT output;
        float4 pos = float4(input.pos, 1.0f);
        pos = mul(pos, worldMatrix);
        pos = mul(pos, viewMatrix);
        pos = mul(pos, projectionMatrix);
        output.pos = pos;
        output.color = input.color;
        return output;
    }
)";

const char* pixelShaderSource = R"(
    float4 main(float4 pos : SV_POSITION, float4 color : COLOR) : SV_TARGET
    {
        return color;
    }
)";
// --- End Embed Shaders ---

Graphics::Graphics() : m_rotation(0.0f) {}

void Graphics::Initialize(HWND hwnd, int width, int height)
{
    // Swap Chain Description
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 0;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;
    sd.OutputWindow = hwnd;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

    ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevels, 1,
        D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, nullptr, &m_deviceContext
    ));

    Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer;
    ThrowIfFailed(m_swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer));
    ThrowIfFailed(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView));

    // Create Depth/Stencil Buffer
    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;
    ThrowIfFailed(m_device->CreateTexture2D(&depthStencilDesc, nullptr, &m_depthStencilBuffer));
    ThrowIfFailed(m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, &m_depthStencilView));

    m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set the viewport
    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<float>(width);
    vp.Height = static_cast<float>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    m_deviceContext->RSSetViewports(1, &vp);

    // Initialize the rendering pipeline
    InitPipeline();

    // Setup projection matrix
    m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV4, // 45 degrees
        static_cast<float>(width) / static_cast<float>(height),
        0.1f,
        100.0f
    );
}

void Graphics::InitPipeline()
{
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, pixelShaderBlob, errorBlob;
    
    // Compile Vertex Shader
    HRESULT hr = D3DCompile(vertexShaderSource, strlen(vertexShaderSource), "VertexShader", nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertexShaderBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) { throw std::runtime_error(std::string("VS Error: ") + (char*)errorBlob->GetBufferPointer()); } else { ThrowIfFailed(hr); } }
    ThrowIfFailed(m_device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &m_vertexShader));

    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ThrowIfFailed(m_device->CreateInputLayout(inputLayoutDesc, ARRAYSIZE(inputLayoutDesc), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &m_inputLayout));

    // Compile Pixel Shader
    hr = D3DCompile(pixelShaderSource, strlen(pixelShaderSource), "PixelShader", nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixelShaderBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) { throw std::runtime_error(std::string("PS Error: ") + (char*)errorBlob->GetBufferPointer()); } else { ThrowIfFailed(hr); } }
    ThrowIfFailed(m_device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &m_pixelShader));

    // Create Vertex Buffer
    Vertex vertices[] = {
        { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3( 0.0f,  0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3( 0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
    };
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    ThrowIfFailed(m_device->CreateBuffer(&bd, new D3D11_SUBRESOURCE_DATA{ vertices, 0, 0 }, &m_vertexBuffer));

    // Create Constant Buffer
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(CB_VS_vertexshader);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ThrowIfFailed(m_device->CreateBuffer(&cbd, nullptr, &m_constantBuffer));
}

void Graphics::RenderFrame()
{
    // Clear the back buffer and depth/stencil view
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Update rotation
    m_rotation += 0.005f;
    if (m_rotation > 6.28f) m_rotation = 0.0f;

    // Update matrices
    m_worldMatrix = DirectX::XMMatrixRotationY(m_rotation);
    m_viewMatrix = DirectX::XMMatrixLookAtLH({ 0.0f, 0.0f, -2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });

    CB_VS_vertexshader cb;
    DirectX::XMStoreFloat4x4(&cb.worldMatrix, DirectX::XMMatrixTranspose(m_worldMatrix));
    DirectX::XMStoreFloat4x4(&cb.viewMatrix, DirectX::XMMatrixTranspose(m_viewMatrix));
    DirectX::XMStoreFloat4x4(&cb.projectionMatrix, DirectX::XMMatrixTranspose(m_projectionMatrix));

    m_deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
    m_deviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    // Set pipeline state
    m_deviceContext->IASetInputLayout(m_inputLayout.Get());
    m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), new UINT{ sizeof(Vertex) }, new UINT{ 0 });
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw the triangle
    m_deviceContext->Draw(3, 0);

    // Present the frame
    ThrowIfFailed(m_swapChain->Present(1, 0));
}
