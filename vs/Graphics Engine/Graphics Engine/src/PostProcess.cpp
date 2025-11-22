#include "include/EnginePCH.h"
#include "include/PostProcess.h"
#include "include/Shader.h"

PostProcess::PostProcess() = default;
PostProcess::~PostProcess() = default;

void PostProcess::Init(ID3D11Device* device, int width, int height)
{
    // 1. Create off-screen texture, RTV, and SRV
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // Use a high-precision format for HDR
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr, &m_offScreenTexture));
    ThrowIfFailed(device->CreateRenderTargetView(m_offScreenTexture.Get(), nullptr, &m_offScreenRTV));
    ThrowIfFailed(device->CreateShaderResourceView(m_offScreenTexture.Get(), nullptr, &m_offScreenSRV));

    // 2. Create shaders
    m_vs = std::make_unique<VertexShader>();
    m_vs->Init(device, L"Assets/Shaders/PostProcess.hlsl", "VS_main", nullptr, 0); // No input layout needed for this VS

    m_ps = std::make_unique<PixelShader>();
    m_ps->Init(device, L"Assets/Shaders/PostProcess.hlsl", "PS_main");

    // 3. Create sampler state
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ThrowIfFailed(device->CreateSamplerState(&sampDesc, &m_sampler));

    // 4. Create Rasterizer State
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;
    ThrowIfFailed(device->CreateRasterizerState(&rsDesc, &m_rsState));
}

void PostProcess::Bind(ID3D11DeviceContext* context, ID3D11DepthStencilView* dsv)
{
    // Set our off-screen texture as the render target
    context->OMSetRenderTargets(1, m_offScreenRTV.GetAddressOf(), dsv);

    // Clear the render target and depth stencil
    const float clearColor[] = { 0.0f, 0.05f, 0.1f, 1.0f }; // Match original scene clear color
    context->ClearRenderTargetView(m_offScreenRTV.Get(), clearColor);
    context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void PostProcess::Draw(ID3D11DeviceContext* context, ID3D11RenderTargetView* backBufferRTV)
{
    // Set the back buffer as the render target
    context->OMSetRenderTargets(1, &backBufferRTV, nullptr); // No depth stencil needed

    // Clear the back buffer
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context->ClearRenderTargetView(backBufferRTV, clearColor);

    // Set states
    context->RSSetState(m_rsState.Get());

    // Bind the off-screen texture as a shader resource
    context->PSSetShaderResources(0, 1, m_offScreenSRV.GetAddressOf());
    context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

    // Bind shaders
    m_vs->Bind(context);
    m_ps->Bind(context);

    // Draw the full-screen triangle
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->Draw(3, 0);

    // Unbind the shader resource and reset states
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    context->PSSetShaderResources(0, 1, nullSRV);
    context->RSSetState(nullptr);
}
