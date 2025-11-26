#include "../../include/Utils/EnginePCH.h"
#include "../../include/Renderer/PostProcess.h"
#include "../../include/ResourceManagement/Shader.h"
#include "../../include/Renderer/BloomEffect.h"

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
    m_vs->Init(device, L"../Assets/Shaders/PostProcess.hlsl", "VS_main", nullptr, 0); // No input layout needed for this VS

    m_ps = std::make_unique<PixelShader>();
    m_ps->Init(device, L"../Assets/Shaders/PostProcess.hlsl", "PS_main");

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
    
    // 5. Initialize Bloom Effect
    m_bloomEffect = std::make_unique<BloomEffect>();
    m_bloomEffect->Init(device, width, height, 0.3f, 0.5f); // Very low threshold, very high intensity for testing
    
    // 6. Create a 1x1 black texture for when bloom is disabled
    // DirectX requires a valid texture - can't pass nullptr
    D3D11_TEXTURE2D_DESC blackTexDesc = {};
    blackTexDesc.Width = 1;
    blackTexDesc.Height = 1;
    blackTexDesc.MipLevels = 1;
    blackTexDesc.ArraySize = 1;
    blackTexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    blackTexDesc.SampleDesc.Count = 1;
    blackTexDesc.Usage = D3D11_USAGE_IMMUTABLE;
    blackTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    
    float blackPixel[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    D3D11_SUBRESOURCE_DATA blackInitData = {};
    blackInitData.pSysMem = blackPixel;
    blackInitData.SysMemPitch = sizeof(float) * 4;
    
    ThrowIfFailed(device->CreateTexture2D(&blackTexDesc, &blackInitData, &m_blackTexture));
    ThrowIfFailed(device->CreateShaderResourceView(m_blackTexture.Get(), nullptr, &m_blackSRV));
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
    // === STEP 1: Apply Bloom Effect (if enabled) ===
    ID3D11ShaderResourceView* bloomSRV;
    
    if (m_bloomEnabled)
    {
        bloomSRV = m_bloomEffect->Apply(context, m_offScreenSRV.Get());
    }
    else
    {
        // Use black texture when bloom is disabled (can't pass nullptr)
        bloomSRV = m_blackSRV.Get();
    }
    
    // === STEP 2: Final Pass (Tone Mapping + Gamma Correction) ===
    
    // Set the back buffer as the render target
    context->OMSetRenderTargets(1, &backBufferRTV, nullptr); // No depth stencil needed

    // Clear the back buffer
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context->ClearRenderTargetView(backBufferRTV, clearColor);

    // Set states
    context->RSSetState(m_rsState.Get());

    // Bind the off-screen texture (scene) and bloom texture as shader resources
    ID3D11ShaderResourceView* srvs[] = { m_offScreenSRV.Get(), bloomSRV }; // bloomSRV can be nullptr
    context->PSSetShaderResources(0, 2, srvs);
    context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

    // Bind shaders
    m_vs->Bind(context);
    m_ps->Bind(context);

    // Draw the full-screen triangle
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->Draw(3, 0);

    // Unbind the shader resources and reset states
    ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
    context->PSSetShaderResources(0, 2, nullSRV);
    context->RSSetState(nullptr);
}

