#include "EnginePCH.h"
#include "Renderer.h"
#include "Graphics.h"
#include "Camera.h"
#include "GameObject.h"
#include "Shader.h"
#include "Skybox.h"
#include "PostProcess.h"
#include "Material.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

Renderer::Renderer() = default;
Renderer::~Renderer() = default;

void Renderer::Initialize(Graphics* graphics, int width, int height)
{
    m_graphics = graphics;
    InitPipeline(width, height);
}

void Renderer::RenderFrame(
    const Camera& camera,
    const std::vector<std::unique_ptr<GameObject>>& gameObjects,
    const DirectionalLight& dirLight,
    const std::vector<PointLight>& pointLights)
{
    ID3D11DeviceContext* context = m_graphics->GetContext().Get();
    ID3D11DepthStencilView* dsv = m_graphics->GetDepthStencilView().Get();
    ID3D11RenderTargetView* rtv = m_graphics->GetRenderTargetView().Get();

    // --- DEBUG: Bypassing Post-Processing to test scene rendering ---
    
    // 1. Render shadows first, as it uses its own render targets
    DirectX::XMMATRIX lightView, lightProj;
    RenderShadowPass(gameObjects, lightView, lightProj);

    // 2. Set main back buffer as render target and clear it
    context->OMSetRenderTargets(1, &rtv, dsv);
    const float clearColor[] = { 0.0f, 0.05f, 0.1f, 1.0f };
    context->ClearRenderTargetView(rtv, clearColor);
    context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // 3. Render scene directly to back buffer
    RenderMainPass(camera, gameObjects, lightView * lightProj, dirLight, pointLights);


    // --- Original Post-Processing Flow (commented out) ---
    /*
    // Step 1: Render shadows first
    DirectX::XMMATRIX lightView, lightProj;
    RenderShadowPass(gameObjects, lightView, lightProj);
    
    // Step 2: Bind the post-process render target for the main pass
    m_postProcess->Bind(context, dsv);

    // Step 3: Render the scene to the off-screen texture.
    RenderMainPass(camera, gameObjects, lightView * lightProj, dirLight, pointLights);

    // Step 4: Apply post-processing and draw to the back buffer.
    m_postProcess->Draw(context, rtv);
    */
}

void Renderer::InitPipeline(int width, int height)
{
    ID3D11Device* device = m_graphics->GetDevice().Get();

    // --- 1. Main Shaders ---
    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    m_mainVS = std::make_unique<VertexShader>();
    m_mainVS->Init(device, L"Shaders/Standard.hlsl", "VS_main", inputLayoutDesc, ARRAYSIZE(inputLayoutDesc));

    m_mainPS = std::make_unique<PixelShader>();
    m_mainPS->Init(device, L"Shaders/Standard.hlsl", "PS_main");

    // --- 2. Shadow Shaders ---
    m_shadowVS = std::make_unique<VertexShader>();
    m_shadowVS->Init(device, L"Shaders/Shadow.hlsl", "main", nullptr, 0);

    // --- 3. Constant Buffers ---
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    cbd.ByteWidth = sizeof(CB_VS_vertexshader);
    ThrowIfFailed(device->CreateBuffer(&cbd, nullptr, &m_vsConstantBuffer));

    cbd.ByteWidth = sizeof(CB_PS_Frame);
    ThrowIfFailed(device->CreateBuffer(&cbd, nullptr, &m_psFrameConstantBuffer));

    cbd.ByteWidth = sizeof(CBuffer_PS_Material);
    ThrowIfFailed(device->CreateBuffer(&cbd, nullptr, &m_psMaterialConstantBuffer));

    cbd.ByteWidth = sizeof(DirectX::XMMATRIX);
    ThrowIfFailed(device->CreateBuffer(&cbd, nullptr, &m_cbShadowMatrix));
    
    // --- 4. Textures & Samplers ---
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
    ThrowIfFailed(device->CreateTexture2D(&texDesc, &texSubData, &texture));
    ThrowIfFailed(device->CreateShaderResourceView(texture.Get(), nullptr, &m_textureView));

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    sampDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
    ThrowIfFailed(device->CreateSamplerState(&sampDesc, &m_samplerState));

    // --- 5. Shadow Map ---
    D3D11_TEXTURE2D_DESC shadowMapDesc = {};
    shadowMapDesc.Width = SHADOW_MAP_SIZE;
    shadowMapDesc.Height = SHADOW_MAP_SIZE;
    shadowMapDesc.MipLevels = 1;
    shadowMapDesc.ArraySize = 1;
    shadowMapDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    shadowMapDesc.SampleDesc.Count = 1;
    shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
    shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    ThrowIfFailed(device->CreateTexture2D(&shadowMapDesc, nullptr, &m_shadowMapTexture));

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    ThrowIfFailed(device->CreateDepthStencilView(m_shadowMapTexture.Get(), &dsvDesc, &m_shadowDSV));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    ThrowIfFailed(device->CreateShaderResourceView(m_shadowMapTexture.Get(), &srvDesc, &m_shadowSRV));

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
    ThrowIfFailed(device->CreateSamplerState(&shadowSampDesc, &m_shadowSampler));

    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.DepthClipEnable = true;
    rsDesc.DepthBias = 10000;
    rsDesc.SlopeScaledDepthBias = 1.0f;
    ThrowIfFailed(device->CreateRasterizerState(&rsDesc, &m_shadowRS));

    // --- 6. Scene objects ---
    m_skybox = std::make_unique<Skybox>();
    m_skybox->Init(device, m_graphics->GetContext().Get(), L"Assets/Textures/sky.jpg");

    m_postProcess = std::make_unique<PostProcess>();
    m_postProcess->Init(device, width, height);

    // --- 7. Matrices ---
    m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV4, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f
    );
}

void Renderer::RenderShadowPass(const std::vector<std::unique_ptr<GameObject>>& gameObjects, DirectX::XMMATRIX& outLightView, DirectX::XMMATRIX& outLightProj)
{
    ID3D11DeviceContext* context = m_graphics->GetContext().Get();

    context->RSSetState(m_shadowRS.Get());

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<float>(SHADOW_MAP_SIZE);
    vp.Height = static_cast<float>(SHADOW_MAP_SIZE);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    context->RSSetViewports(1, &vp);

    context->OMSetRenderTargets(0, nullptr, m_shadowDSV.Get());
    context->ClearDepthStencilView(m_shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    DirectX::XMVECTOR lightPos = DirectX::XMVectorSet(20.0f, 30.0f, -20.0f, 0.0f);
    DirectX::XMVECTOR lightTarget = DirectX::XMVectorZero();
    outLightView = DirectX::XMMatrixLookAtLH(lightPos, lightTarget, { 0.0f, 1.0f, 0.0f });
    outLightProj = DirectX::XMMatrixOrthographicLH(40.0f, 40.0f, 0.1f, 100.0f);

    m_shadowVS->Bind(context);
    context->PSSetShader(nullptr, nullptr, 0);
    context->IASetInputLayout(m_mainVS->GetInputLayout());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (const auto& pGameObject : gameObjects)
    {
        if (!pGameObject) continue;
        DirectX::XMMATRIX worldMatrix = pGameObject->GetWorldMatrix();
        DirectX::XMMATRIX wvp = worldMatrix * outLightView * outLightProj;

        DirectX::XMMATRIX wvpT = DirectX::XMMatrixTranspose(wvp);
        context->UpdateSubresource(m_cbShadowMatrix.Get(), 0, nullptr, &wvpT, 0, 0);
        context->VSSetConstantBuffers(0, 1, m_cbShadowMatrix.GetAddressOf());

        pGameObject->Draw(context, m_psMaterialConstantBuffer.Get());
    }
}

void Renderer::RenderMainPass(
    const Camera& camera,
    const std::vector<std::unique_ptr<GameObject>>& gameObjects,
    const DirectX::XMMATRIX& lightViewProj,
    const DirectionalLight& dirLight,
    const std::vector<PointLight>& pointLights
)
{
    ID3D11DeviceContext* context = m_graphics->GetContext().Get();

    // NOTE: RTV and DSV are set by the caller (RenderFrame via PostProcess::Bind)
    // Viewport is also set by the caller.
    // Let's reset it here just in case.
    D3D11_TEXTURE2D_DESC backBufferDesc;
    m_graphics->GetBackBuffer().Get()->GetDesc(&backBufferDesc);
    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<float>(backBufferDesc.Width);
    vp.Height = static_cast<float>(backBufferDesc.Height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    context->RSSetViewports(1, &vp);

    context->RSSetState(nullptr);
    
    DirectX::XMMATRIX viewMatrix = camera.GetViewMatrix();

    CB_PS_Frame ps_frame_cb;
    ps_frame_cb.dirLight = dirLight;
    for (size_t i = 0; i < 4; ++i)
    {
        if (i < pointLights.size()) {
            ps_frame_cb.pointLights[i] = pointLights[i];
        }
        else {
            ps_frame_cb.pointLights[i] = {};
            ps_frame_cb.pointLights[i].color.w = 0;
        }
    }
    DirectX::XMStoreFloat4(&ps_frame_cb.cameraPos, camera.GetPosition());
    context->UpdateSubresource(m_psFrameConstantBuffer.Get(), 0, nullptr, &ps_frame_cb, 0, 0);

    context->PSSetConstantBuffers(0, 1, m_psFrameConstantBuffer.GetAddressOf());
    context->PSSetShaderResources(0, 1, m_textureView.GetAddressOf());
    context->PSSetShaderResources(2, 1, m_shadowSRV.GetAddressOf());
    context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
    context->PSSetSamplers(2, 1, m_shadowSampler.GetAddressOf());

    m_mainVS->Bind(context);
    m_mainPS->Bind(context);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (const auto& pGameObject : gameObjects)
    {
        if (!pGameObject) continue;
        DirectX::XMMATRIX worldMatrix = pGameObject->GetWorldMatrix();

        CB_VS_vertexshader vs_cb;
        DirectX::XMStoreFloat4x4(&vs_cb.worldMatrix, DirectX::XMMatrixTranspose(worldMatrix));
        DirectX::XMStoreFloat4x4(&vs_cb.viewMatrix, DirectX::XMMatrixTranspose(viewMatrix));
        DirectX::XMStoreFloat4x4(&vs_cb.projectionMatrix, DirectX::XMMatrixTranspose(m_projectionMatrix));
        DirectX::XMStoreFloat4x4(&vs_cb.lightViewProjMatrix, DirectX::XMMatrixTranspose(lightViewProj));
        context->UpdateSubresource(m_vsConstantBuffer.Get(), 0, nullptr, &vs_cb, 0, 0);

        context->VSSetConstantBuffers(0, 1, m_vsConstantBuffer.GetAddressOf());

        pGameObject->Draw(context, m_psMaterialConstantBuffer.Get());
    }

    // Draw Skybox
    if (m_skybox)
    {
        m_skybox->Draw(context, camera, m_projectionMatrix);
    }
}
