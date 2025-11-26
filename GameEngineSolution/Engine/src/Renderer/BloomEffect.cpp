#include "../../include/Utils/EnginePCH.h"
#include "../../include/Renderer/BloomEffect.h"
#include "../../include/ResourceManagement/Shader.h"

BloomEffect::BloomEffect()
    : m_threshold(1.0f)
    , m_intensity(0.04f)
    , m_width(0)
    , m_height(0)
{
}

BloomEffect::~BloomEffect() = default;

void BloomEffect::Init(ID3D11Device* device, int width, int height, float threshold, float intensity)
{
    m_width = width;
    m_height = height;
    m_threshold = threshold;
    m_intensity = intensity;

    // Create texture descriptor for HDR render targets
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // HDR format
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    // 1. Create bright pass texture
    ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr, &m_brightPassTexture));
    ThrowIfFailed(device->CreateRenderTargetView(m_brightPassTexture.Get(), nullptr, &m_brightPassRTV));
    ThrowIfFailed(device->CreateShaderResourceView(m_brightPassTexture.Get(), nullptr, &m_brightPassSRV));

    // 2. Create blur texture 1
    ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr, &m_blurTexture1));
    ThrowIfFailed(device->CreateRenderTargetView(m_blurTexture1.Get(), nullptr, &m_blurRTV1));
    ThrowIfFailed(device->CreateShaderResourceView(m_blurTexture1.Get(), nullptr, &m_blurSRV1));

    // 3. Create blur texture 2
    ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr, &m_blurTexture2));
    ThrowIfFailed(device->CreateRenderTargetView(m_blurTexture2.Get(), nullptr, &m_blurRTV2));
    ThrowIfFailed(device->CreateShaderResourceView(m_blurTexture2.Get(), nullptr, &m_blurSRV2));

    // 4. Create shaders
    m_fullscreenVS = std::make_unique<VertexShader>();
    m_fullscreenVS->Init(device, L"../Assets/Shaders/PostProcess.hlsl", "VS_main", nullptr, 0);

    m_brightPassPS = std::make_unique<PixelShader>();
    m_brightPassPS->Init(device, L"../Assets/Shaders/BrightPass.hlsl", "main");

    m_blurPS = std::make_unique<PixelShader>();
    m_blurPS->Init(device, L"../Assets/Shaders/GaussianBlur.hlsl", "main");

    // 5. Create sampler state
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ThrowIfFailed(device->CreateSamplerState(&sampDesc, &m_sampler));

    // 6. Create constant buffer for blur parameters
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(BlurParams);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ThrowIfFailed(device->CreateBuffer(&cbDesc, nullptr, &m_blurParamsCB));
}

ID3D11ShaderResourceView* BloomEffect::Apply(ID3D11DeviceContext* context, ID3D11ShaderResourceView* sourceSRV)
{
    // === STEP 1: Bright Pass Extraction ===
    {
        // Set bright pass render target
        context->OMSetRenderTargets(1, m_brightPassRTV.GetAddressOf(), nullptr);

        // Clear
        const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        context->ClearRenderTargetView(m_brightPassRTV.Get(), clearColor);

        // Update constant buffer with threshold
        BlurParams params;
        params.direction = DirectX::XMFLOAT2(0.0f, 0.0f); // Not used in bright pass
        params.threshold = m_threshold;
        params.padding = 0.0f;
        context->UpdateSubresource(m_blurParamsCB.Get(), 0, nullptr, &params, 0, 0);
        context->PSSetConstantBuffers(0, 1, m_blurParamsCB.GetAddressOf());

        // Bind source texture and sampler
        context->PSSetShaderResources(0, 1, &sourceSRV);
        context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

        // Draw full-screen triangle
        m_fullscreenVS->Bind(context);
        m_brightPassPS->Bind(context);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->Draw(3, 0);

        // Unbind
        ID3D11ShaderResourceView* nullSRV = nullptr;
        context->PSSetShaderResources(0, 1, &nullSRV);
    }

    // === STEP 2: Horizontal Blur ===
    {
        // Set blur texture 1 as render target
        context->OMSetRenderTargets(1, m_blurRTV1.GetAddressOf(), nullptr);

        const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        context->ClearRenderTargetView(m_blurRTV1.Get(), clearColor);

        // Update constant buffer for horizontal blur
        BlurParams params;
        params.direction = DirectX::XMFLOAT2(1.0f / m_width, 0.0f);
        params.threshold = m_threshold;
        params.padding = 0.0f;
        context->UpdateSubresource(m_blurParamsCB.Get(), 0, nullptr, &params, 0, 0);
        context->PSSetConstantBuffers(0, 1, m_blurParamsCB.GetAddressOf());

        // Bind bright pass texture
        context->PSSetShaderResources(0, 1, m_brightPassSRV.GetAddressOf());
        context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

        // Draw
        m_fullscreenVS->Bind(context);
        m_blurPS->Bind(context);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->Draw(3, 0);

        // Unbind
        ID3D11ShaderResourceView* nullSRV = nullptr;
        context->PSSetShaderResources(0, 1, &nullSRV);
    }

    // === STEP 3: Vertical Blur ===
    {
        // Set blur texture 2 as render target
        context->OMSetRenderTargets(1, m_blurRTV2.GetAddressOf(), nullptr);

        const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        context->ClearRenderTargetView(m_blurRTV2.Get(), clearColor);

        // Update constant buffer for vertical blur
        BlurParams params;
        params.direction = DirectX::XMFLOAT2(0.0f, 1.0f / m_height);
        params.threshold = m_threshold;
        params.padding = 0.0f;
        context->UpdateSubresource(m_blurParamsCB.Get(), 0, nullptr, &params, 0, 0);
        context->PSSetConstantBuffers(0, 1, m_blurParamsCB.GetAddressOf());

        // Bind horizontally blurred texture
        context->PSSetShaderResources(0, 1, m_blurSRV1.GetAddressOf());
        context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

        // Draw
        m_fullscreenVS->Bind(context);
        m_blurPS->Bind(context);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->Draw(3, 0);

        // Unbind
        ID3D11ShaderResourceView* nullSRV = nullptr;
        context->PSSetShaderResources(0, 1, &nullSRV);
    }

    // Return the final blurred texture
    return m_blurSRV2.Get();
}
