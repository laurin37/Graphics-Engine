#include "EnginePCH.h"
#include "Graphics.h"
#include "GameObject.h"
#include "Camera.h"
#include "Mesh.h"
#include "Material.h"
#include "Skybox.h"
#include <vector>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// --- SHADER SOURCES ---

const char* uiVertexShaderSource = R"(
    cbuffer ConstantBuffer : register(b0)
    {
        float2 screenSize;
        float2 padding;
    }

    struct VS_INPUT
    {
        float3 pos : POSITION;
        float2 uv : TEXCOORD;
        float4 color : COLOR;
    };

    struct PS_INPUT
    {
        float4 pos : SV_POSITION;
        float2 uv : TEXCOORD;
        float4 color : COLOR;
    };

    PS_INPUT main(VS_INPUT input)
    {
        PS_INPUT output;
        
        // Convert Screen Pixels to NDC (-1 to 1)
        output.pos.x = (input.pos.x / screenSize.x) * 2.0 - 1.0;
        output.pos.y = -((input.pos.y / screenSize.y) * 2.0 - 1.0);
        
        // FIX: Set Z to 0.1 instead of 0.0 to avoid Near-Plane clipping
        output.pos.z = 0.1; 
        output.pos.w = 1.0;

        output.uv = input.uv;
        output.color = input.color;
        return output;
    }
)";

const char* uiPixelShaderSource = R"(
    Texture2D g_texture : register(t0);
    SamplerState g_sampler : register(s0);

    struct PS_INPUT
    {
        float4 pos : SV_POSITION;
        float2 uv : TEXCOORD;
        float4 color : COLOR;
    };

    float4 main(PS_INPUT input) : SV_TARGET
    {
        float4 texColor = g_texture.Sample(g_sampler, input.uv);
        
        // DEBUG: If texture alpha is missing, draw RED block so we see SOMETHING.
        if (texColor.a < 0.1) return float4(1, 0, 0, 0.5) * input.color; 
        
        return texColor * input.color;
    }
)";



Graphics::Graphics()
    : m_projectionMatrix(DirectX::XMMatrixIdentity())
    , m_screenWidth(0), m_screenHeight(0)
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

    InitPipeline();

    m_skybox = std::make_unique<Skybox>();
    m_skybox->Init(m_device.Get(), m_deviceContext.Get(), L"Assets/Textures/sky.jpg");

    m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV4, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f
    );
}

void Graphics::InitPipeline()
{
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob, uiVSBlob, uiPSBlob;

    // --- 1. Main Shaders ---
    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    m_mainVS = std::make_unique<VertexShader>();
    m_mainVS->Init(m_device.Get(), L"Shaders/Standard.hlsl", "VS_main", inputLayoutDesc, ARRAYSIZE(inputLayoutDesc));

    m_mainPS = std::make_unique<PixelShader>();
    m_mainPS->Init(m_device.Get(), L"Shaders/Standard.hlsl", "PS_main");

    // --- 2. Shadow Shaders ---
    m_shadowVS = std::make_unique<VertexShader>();
    m_shadowVS->Init(m_device.Get(), L"Shaders/Shadow.hlsl", "main", nullptr, 0);


    // --- 3. UI Shaders ---
    HRESULT hr = D3DCompile(uiVertexShaderSource, strlen(uiVertexShaderSource), "UIVS", nullptr, nullptr, "main", "vs_5_0", 0, 0, &uiVSBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) { throw std::runtime_error(std::string("UI VS Error: ") + (char*)errorBlob->GetBufferPointer()); } else { ThrowIfFailed(hr); } }
    ThrowIfFailed(m_device->CreateVertexShader(uiVSBlob->GetBufferPointer(), uiVSBlob->GetBufferSize(), nullptr, &m_uiVS));

    hr = D3DCompile(uiPixelShaderSource, strlen(uiPixelShaderSource), "UIPS", nullptr, nullptr, "main", "ps_5_0", 0, 0, &uiPSBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) { throw std::runtime_error(std::string("UI PS Error: ") + (char*)errorBlob->GetBufferPointer()); } else { ThrowIfFailed(hr); } }
    ThrowIfFailed(m_device->CreatePixelShader(uiPSBlob->GetBufferPointer(), uiPSBlob->GetBufferSize(), nullptr, &m_uiPS));

    D3D11_INPUT_ELEMENT_DESC uiInputDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ThrowIfFailed(m_device->CreateInputLayout(uiInputDesc, ARRAYSIZE(uiInputDesc), uiVSBlob->GetBufferPointer(), uiVSBlob->GetBufferSize(), &m_uiInputLayout));

    // --- 4. Constant Buffers ---
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    cbd.ByteWidth = sizeof(CB_VS_vertexshader);
    ThrowIfFailed(m_device->CreateBuffer(&cbd, nullptr, &m_vsConstantBuffer));

    cbd.ByteWidth = sizeof(CB_PS_Frame);
    ThrowIfFailed(m_device->CreateBuffer(&cbd, nullptr, &m_psFrameConstantBuffer));

    cbd.ByteWidth = sizeof(CBuffer_PS_Material);
    ThrowIfFailed(m_device->CreateBuffer(&cbd, nullptr, &m_psMaterialConstantBuffer));

    cbd.ByteWidth = sizeof(DirectX::XMMATRIX);
    ThrowIfFailed(m_device->CreateBuffer(&cbd, nullptr, &m_cbShadowMatrix));

    cbd.ByteWidth = sizeof(CB_VS_UI);
    ThrowIfFailed(m_device->CreateBuffer(&cbd, nullptr, &m_uiConstantBuffer));

    // --- 5. Vertex Buffers (Dynamic for UI) ---
    D3D11_BUFFER_DESC uiVBD = {};
    uiVBD.Usage = D3D11_USAGE_DYNAMIC;
    uiVBD.ByteWidth = sizeof(SpriteVertex) * 256 * 6;
    uiVBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    uiVBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ThrowIfFailed(m_device->CreateBuffer(&uiVBD, nullptr, &m_uiVertexBuffer));

    // --- 6. Textures & Samplers ---
    const int texWidth = 1, texHeight = 1;
    std::vector<uint32_t> texData(texWidth * texHeight, 0xFFFFFFFF);

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

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    sampDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
    ThrowIfFailed(m_device->CreateSamplerState(&sampDesc, &m_samplerState));

    // --- 7. Shadow Map ---
    D3D11_TEXTURE2D_DESC shadowMapDesc = {};
    shadowMapDesc.Width = SHADOW_MAP_SIZE;
    shadowMapDesc.Height = SHADOW_MAP_SIZE;
    shadowMapDesc.MipLevels = 1;
    shadowMapDesc.ArraySize = 1;
    shadowMapDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    shadowMapDesc.SampleDesc.Count = 1;
    shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
    shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    ThrowIfFailed(m_device->CreateTexture2D(&shadowMapDesc, nullptr, &m_shadowMapTexture));

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    ThrowIfFailed(m_device->CreateDepthStencilView(m_shadowMapTexture.Get(), &dsvDesc, &m_shadowDSV));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    ThrowIfFailed(m_device->CreateShaderResourceView(m_shadowMapTexture.Get(), &srvDesc, &m_shadowSRV));

    D3D11_SAMPLER_DESC shadowSampDesc = {};
    shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    shadowSampDesc.BorderColor[0] = 1.0f;
    shadowSampDesc.BorderColor[1] = 1.0f;
    shadowSampDesc.BorderColor[2] = 1.0f;
    shadowSampDesc.BorderColor[3] = 1.0f;
    ThrowIfFailed(m_device->CreateSamplerState(&shadowSampDesc, &m_shadowSampler));

    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.DepthClipEnable = true;
    rsDesc.DepthBias = 10000;
    rsDesc.SlopeScaledDepthBias = 1.0f;
    ThrowIfFailed(m_device->CreateRasterizerState(&rsDesc, &m_shadowRS));

    // --- 8. UI States ---
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

    // --- 9. UI Rasterizer State (No Culling) ---
    D3D11_RASTERIZER_DESC uiRSDesc = {};
    uiRSDesc.FillMode = D3D11_FILL_SOLID;
    uiRSDesc.CullMode = D3D11_CULL_NONE; // Critical for 2D
    uiRSDesc.DepthClipEnable = true;
    ThrowIfFailed(m_device->CreateRasterizerState(&uiRSDesc, &m_uiRS));
}

ID3D11Device* Graphics::GetDevice() const
{
    return m_device.Get();
}

ID3D11DeviceContext* Graphics::GetContext() const
{
    return m_deviceContext.Get();
}

Mesh* Graphics::GetMeshAsset() const
{
    return m_meshAsset.get();
}

void Graphics::RenderFrame(
    Camera* camera,
    const std::vector<std::unique_ptr<GameObject>>& gameObjects,
    const DirectionalLight& dirLight,
    const std::vector<PointLight>& pointLights
)
{
    DirectX::XMMATRIX lightView, lightProj;
    RenderShadowPass(gameObjects, lightView, lightProj);
    RenderMainPass(camera, gameObjects, lightView * lightProj, dirLight, pointLights);

    // Removed: ThrowIfFailed(m_swapChain->Present(1, 0)); 
    // Presentation is now manual via Present()
}

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
    m_deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    m_deviceContext->Draw(static_cast<UINT>(count), 0);
}

void Graphics::RenderShadowPass(const std::vector<std::unique_ptr<GameObject>>& gameObjects, DirectX::XMMATRIX& outLightView, DirectX::XMMATRIX& outLightProj)
{
    m_deviceContext->RSSetState(m_shadowRS.Get());

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<float>(SHADOW_MAP_SIZE);
    vp.Height = static_cast<float>(SHADOW_MAP_SIZE);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    m_deviceContext->RSSetViewports(1, &vp);

    m_deviceContext->OMSetRenderTargets(0, nullptr, m_shadowDSV.Get());
    m_deviceContext->ClearDepthStencilView(m_shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    DirectX::XMVECTOR lightPos = DirectX::XMVectorSet(20.0f, 30.0f, -20.0f, 0.0f);
    DirectX::XMVECTOR lightTarget = DirectX::XMVectorZero();
    outLightView = DirectX::XMMatrixLookAtLH(lightPos, lightTarget, { 0.0f, 1.0f, 0.0f });
    outLightProj = DirectX::XMMatrixOrthographicLH(40.0f, 40.0f, 0.1f, 100.0f);

    m_shadowVS->Bind(m_deviceContext.Get());
    m_deviceContext->PSSetShader(nullptr, nullptr, 0);
    m_deviceContext->IASetInputLayout(m_mainVS->GetInputLayout());
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (const auto& pGameObject : gameObjects)
    {
        if (!pGameObject) continue;
        DirectX::XMMATRIX worldMatrix = pGameObject->GetWorldMatrix();
        DirectX::XMMATRIX wvp = worldMatrix * outLightView * outLightProj;

        DirectX::XMMATRIX wvpT = DirectX::XMMatrixTranspose(wvp);
        m_deviceContext->UpdateSubresource(m_cbShadowMatrix.Get(), 0, nullptr, &wvpT, 0, 0);
        m_deviceContext->VSSetConstantBuffers(0, 1, m_cbShadowMatrix.GetAddressOf());

        pGameObject->Draw(m_deviceContext.Get(), m_psMaterialConstantBuffer.Get());
    }
}

void Graphics::RenderMainPass(
    Camera* camera,
    const std::vector<std::unique_ptr<GameObject>>& gameObjects,
    const DirectX::XMMATRIX& lightViewProj,
    const DirectionalLight& dirLight,
    const std::vector<PointLight>& pointLights
)
{
    D3D11_TEXTURE2D_DESC backBufferDesc;
    Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer;
    m_swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer);
    ((ID3D11Texture2D*)backBuffer.Get())->GetDesc(&backBufferDesc);

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<float>(backBufferDesc.Width);
    vp.Height = static_cast<float>(backBufferDesc.Height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    m_deviceContext->RSSetViewports(1, &vp);

    m_deviceContext->RSSetState(nullptr);

    m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
    const float clearColor[] = { 0.0f, 0.05f, 0.1f, 1.0f };
    m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    DirectX::XMMATRIX viewMatrix = camera->GetViewMatrix();

    CB_PS_Frame ps_frame_cb;
    ps_frame_cb.dirLight = dirLight;
    for (size_t i = 0; i < MAX_POINT_LIGHTS; ++i)
    {
        if (i < pointLights.size()) {
            ps_frame_cb.pointLights[i] = pointLights[i];
        }
        else {
            ps_frame_cb.pointLights[i] = {};
            ps_frame_cb.pointLights[i].color.w = 0;
        }
    }
    DirectX::XMStoreFloat4(&ps_frame_cb.cameraPos, camera->GetPosition());
    m_deviceContext->UpdateSubresource(m_psFrameConstantBuffer.Get(), 0, nullptr, &ps_frame_cb, 0, 0);

    m_deviceContext->PSSetConstantBuffers(0, 1, m_psFrameConstantBuffer.GetAddressOf());
    m_deviceContext->PSSetShaderResources(0, 1, m_textureView.GetAddressOf());
    m_deviceContext->PSSetShaderResources(2, 1, m_shadowSRV.GetAddressOf());
    m_deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
    m_deviceContext->PSSetSamplers(2, 1, m_shadowSampler.GetAddressOf());

    m_mainVS->Bind(m_deviceContext.Get());
    m_mainPS->Bind(m_deviceContext.Get());
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (const auto& pGameObject : gameObjects)
    {
        if (!pGameObject) continue;
        DirectX::XMMATRIX worldMatrix = pGameObject->GetWorldMatrix();

        CB_VS_vertexshader vs_cb;
        DirectX::XMStoreFloat4x4(&vs_cb.worldMatrix, DirectX::XMMatrixTranspose(worldMatrix));
        DirectX::XMStoreFloat4x4(&vs_cb.viewMatrix, DirectX::XMMatrixTranspose(viewMatrix));
        DirectX::XMStoreFloat4x4(&vs_cb.projectionMatrix, DirectX::XMMatrixTranspose(m_projectionMatrix));
        DirectX::XMStoreFloat4x4(&vs_cb.lightViewProjMatrix, DirectX::XMMatrixTranspose(lightViewProj));
        m_deviceContext->UpdateSubresource(m_vsConstantBuffer.Get(), 0, nullptr, &vs_cb, 0, 0);

        pGameObject->Draw(m_deviceContext.Get(), m_psMaterialConstantBuffer.Get());
    }

    // Draw Skybox
    if (m_skybox)
    {
        m_skybox->Draw(m_deviceContext.Get(), *camera, m_projectionMatrix);
    }
}