#include "EnginePCH.h"
#include "Graphics.h"
#include "GameObject.h"
#include "Camera.h"
#include "Mesh.h"
#include "Material.h"
#include "Skybox.h"
#include "PostProcess.h"
#include <vector>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

Graphics::Graphics()
    : m_screenWidth(0), m_screenHeight(0)
{
}

Graphics::~Graphics() = default;

void Graphics::Initialize(HWND hwnd, int width, int height)
{
    m_screenWidth = static_cast<float>(width);
    m_screenHeight = static_cast<float>(height);

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

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevels, 1,
        D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, nullptr, &m_deviceContext
    ));

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
    m_backBufferTexture = backBuffer;
    ThrowIfFailed(m_device->CreateRenderTargetView(m_backBufferTexture.Get(), nullptr, &m_renderTargetView));

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

    InitUI();
}

void Graphics::InitUI()
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> uiVSBlob, uiPSBlob, errorBlob;

    // Compile Vertex Shader
    HRESULT hr = D3DCompileFromFile(L"Shaders/UIVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", flags, 0, &uiVSBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            // Throw an exception with the detailed error message
            std::string errorMessage = "UI Vertex Shader compilation failed: ";
            errorMessage += (char*)errorBlob->GetBufferPointer();
            throw std::runtime_error(errorMessage);
        }
        else
        {
            ThrowIfFailed(hr); // Handle other errors like file not found
        }
    }
    ThrowIfFailed(m_device->CreateVertexShader(uiVSBlob->GetBufferPointer(), uiVSBlob->GetBufferSize(), nullptr, &m_uiVS));

    // Compile Pixel Shader
    hr = D3DCompileFromFile(L"Shaders/UIPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", flags, 0, &uiPSBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            std::string errorMessage = "UI Pixel Shader compilation failed: ";
            errorMessage += (char*)errorBlob->GetBufferPointer();
            throw std::runtime_error(errorMessage);
        }
        else
        {
            ThrowIfFailed(hr);
        }
    }
    ThrowIfFailed(m_device->CreatePixelShader(uiPSBlob->GetBufferPointer(), uiPSBlob->GetBufferSize(), nullptr, &m_uiPS));

    D3D11_INPUT_ELEMENT_DESC uiInputDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ThrowIfFailed(m_device->CreateInputLayout(uiInputDesc, ARRAYSIZE(uiInputDesc), uiVSBlob->GetBufferPointer(), uiVSBlob->GetBufferSize(), &m_uiInputLayout));

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.ByteWidth = sizeof(CB_VS_UI);
    ThrowIfFailed(m_device->CreateBuffer(&cbd, nullptr, &m_uiConstantBuffer));

    D3D11_BUFFER_DESC uiVBD = {};
    uiVBD.Usage = D3D11_USAGE_DYNAMIC;
    uiVBD.ByteWidth = sizeof(SpriteVertex) * 256 * 6;
    uiVBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    uiVBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ThrowIfFailed(m_device->CreateBuffer(&uiVBD, nullptr, &m_uiVertexBuffer));

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    ThrowIfFailed(m_device->CreateBlendState(&blendDesc, &m_uiBlendState));

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = FALSE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    ThrowIfFailed(m_device->CreateDepthStencilState(&dsDesc, &m_uiDepthStencilState));

    D3D11_RASTERIZER_DESC uiRSDesc = {};
    uiRSDesc.FillMode = D3D11_FILL_SOLID;
    uiRSDesc.CullMode = D3D11_CULL_NONE;
    uiRSDesc.DepthClipEnable = true;
    ThrowIfFailed(m_device->CreateRasterizerState(&uiRSDesc, &m_uiRS));

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ThrowIfFailed(m_device->CreateSamplerState(&sampDesc, &m_uiSamplerState));
}

Microsoft::WRL::ComPtr<ID3D11Device> Graphics::GetDevice() const { return m_device; }
Microsoft::WRL::ComPtr<ID3D11DeviceContext> Graphics::GetContext() const { return m_deviceContext; }
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> Graphics::GetRenderTargetView() const { return m_renderTargetView; }
Microsoft::WRL::ComPtr<ID3D11DepthStencilView> Graphics::GetDepthStencilView() const { return m_depthStencilView; }
Microsoft::WRL::ComPtr<ID3D11Texture2D> Graphics::GetBackBuffer() const { return m_backBufferTexture; }





void Graphics::Present()
{
    ThrowIfFailed(m_swapChain->Present(1, 0));
}

void Graphics::EnableUIState()
{
    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_deviceContext->OMSetBlendState(m_uiBlendState.Get(), blendFactor, 0xffffffff);
    m_deviceContext->OMSetDepthStencilState(m_uiDepthStencilState.Get(), 0);
    m_deviceContext->RSSetState(m_uiRS.Get()); // Set CullNone

    CB_VS_UI cbUI;
    cbUI.screenSize = DirectX::XMFLOAT2(m_screenWidth, m_screenHeight);
    cbUI.padding = DirectX::XMFLOAT2(0, 0);
    m_deviceContext->UpdateSubresource(m_uiConstantBuffer.Get(), 0, nullptr, &cbUI, 0, 0);
    m_deviceContext->VSSetConstantBuffers(0, 1, m_uiConstantBuffer.GetAddressOf());
}

void Graphics::DisableUIState()
{
    m_deviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    m_deviceContext->OMSetDepthStencilState(nullptr, 0);
    m_deviceContext->RSSetState(nullptr); // Reset to default
}

void Graphics::DrawUI(const SpriteVertex* vertices, size_t count, ID3D11ShaderResourceView* texture)
{
    if (count == 0) return;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = m_deviceContext->Map(m_uiVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr)) return;

    memcpy(mappedResource.pData, vertices, sizeof(SpriteVertex) * count);
    m_deviceContext->Unmap(m_uiVertexBuffer.Get(), 0);

    UINT stride = sizeof(SpriteVertex);
    UINT offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, m_uiVertexBuffer.GetAddressOf(), &stride, &offset);
    m_deviceContext->IASetInputLayout(m_uiInputLayout.Get());
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_deviceContext->VSSetShader(m_uiVS.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_uiPS.Get(), nullptr, 0);
    m_deviceContext->PSSetShaderResources(0, 1, &texture);
    m_deviceContext->PSSetSamplers(0, 1, m_uiSamplerState.GetAddressOf());

    m_deviceContext->Draw(static_cast<UINT>(count), 0);
}


