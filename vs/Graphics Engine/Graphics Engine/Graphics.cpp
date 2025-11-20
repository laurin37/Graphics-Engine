#include "EnginePCH.h"
#include "Graphics.h"
#include "GameObject.h"
#include "Camera.h"
#include "Mesh.h"
#include <vector>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

const char* shadowVertexShaderSource = R"(
    cbuffer LightMatrixBuffer : register(b0)
    {
        matrix lightWVP;
    }

    // Only need position for depth pass
    float4 main(float3 pos : POSITION) : SV_POSITION
    {
        return mul(float4(pos, 1.0f), lightWVP);
    }
)";

const char* vertexShaderSource = R"(
    cbuffer ConstantBuffer : register(b0)
    {
        matrix worldMatrix;
        matrix viewMatrix;
        matrix projectionMatrix;
        matrix lightViewProjMatrix;
    }
    struct VS_INPUT { float3 pos : POSITION; float2 uv : TEXCOORD; float3 normal : NORMAL; float3 tangent : TANGENT; };
    struct PS_INPUT { 
        float4 pos : SV_POSITION; 
        float2 uv : TEXCOORD; 
        float3 normal : NORMAL; 
        float3 tangent : TANGENT;
        float3 worldPos : WORLD_POS;
        float4 lightSpacePos : TEXCOORD1;
    };

    PS_INPUT main(VS_INPUT input)
    {
        PS_INPUT output;
        float4 worldPos = mul(float4(input.pos, 1.0f), worldMatrix);
        output.worldPos = worldPos.xyz;
        
        output.pos = mul(worldPos, viewMatrix);
        output.pos = mul(output.pos, projectionMatrix);
        
        output.normal = normalize(mul(input.normal, (float3x3)worldMatrix));
        output.tangent = normalize(mul(input.tangent, (float3x3)worldMatrix));
        output.uv = input.uv;
        output.lightSpacePos = mul(worldPos, lightViewProjMatrix);
        return output;
    }
)";

const char* pixelShaderSource = R"(
    Texture2D g_texture : register(t0);
    Texture2D g_normalMap : register(t1);
    Texture2D g_shadowMap : register(t2);

    SamplerState g_sampler : register(s0);
    SamplerComparisonState g_shadowSampler : register(s2);


    cbuffer FrameData : register(b0)
    {
        // Directional Light
        float4 dirLightDirection;
        float4 dirLightColor;

        // Point Lights
        float4 pointLightPos[4];
        float4 pointLightColor[4];
        float4 pointLightAtt[4];

        // Camera
        float4 cameraPos;
    }

    cbuffer MaterialData : register(b1)
    {
        float4 surfaceColor;
        float specularIntensity;
        float specularPower;
    }

    struct PS_INPUT { 
        float4 pos : SV_POSITION; 
        float2 uv : TEXCOORD; 
        float3 normal : NORMAL; 
        float3 tangent : TANGENT;
        float3 worldPos : WORLD_POS;
        float4 lightSpacePos : TEXCOORD1;
    };

    // --- Helper Function ---
    float3 CalcLighting(float3 litColor, float3 pixelNormal, float3 lightVec, float3 lightColor, float3 viewDir, float specIntensity, float specPower)
    {
        // Diffuse
        float diffuseFactor = saturate(dot(pixelNormal, lightVec));
        float3 diffuse = litColor * diffuseFactor * lightColor;

        // Specular (Blinn-Phong)
        float3 halfVector = normalize(lightVec + viewDir);
        float specFactor = pow(saturate(dot(pixelNormal, halfVector)), specPower);
        float3 specular = specIntensity * specFactor * lightColor;

        return diffuse + specular;
    }


    float4 main(PS_INPUT input) : SV_TARGET
    {
        // --- Normal Mapping with Auto-Detection ---
        float3 N = normalize(input.normal);
        float3 T = normalize(input.tangent - dot(input.tangent, N) * N);
        float3 B = cross(N, T);
        float3x3 TBN = float3x3(T, B, N);
        
        float3 normalMapSample = g_normalMap.Sample(g_sampler, input.uv).rgb;
        float3 pixelNormal;

        // Check Blue channel. If it's very low (< 0.2), this is likely a grayscale displacement map, not a normal map.
        if (normalMapSample.b < 0.2) 
        {
             // Fallback: Perturb Normal using derivatives of height (R channel)
             float height = normalMapSample.r;
             
             // Calculate derivatives of height and position
             float dHdx = ddx(height);
             float dHdy = ddy(height);
             float3 dPdx = ddx(input.worldPos);
             float3 dPdy = ddy(input.worldPos);

             // Calculate Surface Gradient
             // The factor 50.0 is the 'bump scale' - adjustable for strength
             float3 surfGrad = (dHdx * dPdx + dHdy * dPdy) * 50.0f;
             
             // Perturb the geometric normal
             pixelNormal = normalize(N - surfGrad);
        }
        else
        {
             // Standard Normal Map (Blue > 0.5 usually)
             float3 tangentSpaceNormal = normalize(normalMapSample * 2.0 - 1.0);
             pixelNormal = normalize(mul(tangentSpaceNormal, TBN));
        }

        // VISUAL DEBUG: Uncomment to see the normals directly
        // return float4(pixelNormal * 0.5 + 0.5, 1.0);

        float4 texColor = g_texture.Sample(g_sampler, input.uv);
        float3 baseColor = texColor.rgb * surfaceColor.rgb;
        
        // --- Shadow Calculation ---
        float shadowFactor = 0.0;
        float3 projCoords = input.lightSpacePos.xyz / input.lightSpacePos.w;
        projCoords.x = projCoords.x * 0.5 + 0.5;
        projCoords.y = projCoords.y * -0.5 + 0.5;

        float shadowBias = 0.0005f;
        float texelSize = 1.0 / 2048.0;

        if (saturate(projCoords.z) > 0.0 && saturate(projCoords.x) > 0.0 && saturate(projCoords.x) < 1.0 && saturate(projCoords.y) > 0.0 && saturate(projCoords.y) < 1.0)
        {
            [unroll]
            for (int x = -1; x <= 1; ++x)
            {
                [unroll]
                for (int y = -1; y <= 1; ++y)
                {
                    shadowFactor += g_shadowMap.SampleCmpLevelZero(g_shadowSampler, projCoords.xy + float2(x, y) * texelSize, projCoords.z - shadowBias);
                }
            }
            shadowFactor /= 9.0;
        }
        else
        {
            shadowFactor = 1.0;
        }
        
        // --- Lighting ---
        float3 finalColor = float3(0, 0, 0);
        float3 viewDir = normalize(cameraPos.xyz - input.worldPos);

        // 1. Ambient Light
        finalColor += baseColor * 0.15f;

        // 2. Directional Light (with shadows)
        float3 dirLightContrib = CalcLighting(baseColor, pixelNormal, -dirLightDirection.xyz, dirLightColor.rgb * dirLightColor.a, viewDir, specularIntensity, specularPower);
        finalColor += dirLightContrib * shadowFactor;

        // 3. Point Lights
        [unroll]
        for (int i = 0; i < 4; ++i)
        {
            float3 lightVec = pointLightPos[i].xyz - input.worldPos;
            float dist = length(lightVec);
            
            // Only calculate if within range
            if (dist < pointLightPos[i].w)
            {
                lightVec = normalize(lightVec);

                float3 pointLightContrib = CalcLighting(baseColor, pixelNormal, lightVec, pointLightColor[i].rgb * pointLightColor[i].a, viewDir, specularIntensity, specularPower);
                
                // Attenuation
                float att = 1.0 / (pointLightAtt[i].x + pointLightAtt[i].y * dist + pointLightAtt[i].z * dist * dist);
                finalColor += pointLightContrib * att;
            }
        }

        return float4(finalColor, texColor.a);
    }
)";

Graphics::Graphics() {}

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

    m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV4, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f
    );
}

void Graphics::InitPipeline()
{
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, pixelShaderBlob, errorBlob, shadowVSBlob;

    HRESULT hr = D3DCompile(vertexShaderSource, strlen(vertexShaderSource), "VertexShader", nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertexShaderBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) { throw std::runtime_error(std::string("VS Error: ") + (char*)errorBlob->GetBufferPointer()); } else { ThrowIfFailed(hr); } }
    ThrowIfFailed(m_device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &m_vertexShader));

    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ThrowIfFailed(m_device->CreateInputLayout(inputLayoutDesc, ARRAYSIZE(inputLayoutDesc), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &m_inputLayout));

    hr = D3DCompile(pixelShaderSource, strlen(pixelShaderSource), "PixelShader", nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixelShaderBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) { throw std::runtime_error(std::string("PS Error: ") + (char*)errorBlob->GetBufferPointer()); } else { ThrowIfFailed(hr); } }
    ThrowIfFailed(m_device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &m_pixelShader));

    hr = D3DCompile(shadowVertexShaderSource, strlen(shadowVertexShaderSource), "ShadowVS", nullptr, nullptr, "main", "vs_5_0", 0, 0, &shadowVSBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) { throw std::runtime_error(std::string("Shadow VS Error: ") + (char*)errorBlob->GetBufferPointer()); } else { ThrowIfFailed(hr); } }
    ThrowIfFailed(m_device->CreateVertexShader(shadowVSBlob->GetBufferPointer(), shadowVSBlob->GetBufferSize(), nullptr, &m_shadowVS));

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

    // Create a 1x1 white texture to act as a default for non-textured materials
    const int texWidth = 1, texHeight = 1;
    std::vector<uint32_t> texData(texWidth * texHeight, 0xFFFFFFFF); // 0xFFFFFFFF = white

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
    ThrowIfFailed(m_swapChain->Present(1, 0));
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
    outLightView = DirectX::XMMatrixLookAtLH(lightPos, lightTarget, { 0.0f, 1.0f, 0.0f, 0.0f });
    outLightProj = DirectX::XMMatrixOrthographicLH(40.0f, 40.0f, 0.1f, 100.0f);

    m_deviceContext->VSSetShader(m_shadowVS.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(nullptr, nullptr, 0);
    m_deviceContext->IASetInputLayout(m_inputLayout.Get());
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
    // Get window size to set viewport
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
    const float clearColor[] = { 0.0f, 0.05f, 0.1f, 1.0f }; // Darker blue
    m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    DirectX::XMMATRIX viewMatrix = camera->GetViewMatrix();

    // --- Update Frame Constant Buffer ---
    CB_PS_Frame ps_frame_cb;
    ps_frame_cb.dirLight = dirLight;
    for (size_t i = 0; i < MAX_POINT_LIGHTS; ++i)
    {
        if (i < pointLights.size()) {
            ps_frame_cb.pointLights[i] = pointLights[i];
        }
        else {
            // Zero out unused lights
            ps_frame_cb.pointLights[i] = {};
            ps_frame_cb.pointLights[i].color.w = 0; // Intensity = 0
        }
    }
    DirectX::XMStoreFloat4(&ps_frame_cb.cameraPos, camera->GetPosition());
    m_deviceContext->UpdateSubresource(m_psFrameConstantBuffer.Get(), 0, nullptr, &ps_frame_cb, 0, 0);

    m_deviceContext->PSSetConstantBuffers(0, 1, m_psFrameConstantBuffer.GetAddressOf());
    m_deviceContext->PSSetShaderResources(0, 1, m_textureView.GetAddressOf());
    m_deviceContext->PSSetShaderResources(2, 1, m_shadowSRV.GetAddressOf()); // Slot 2 now
    m_deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
    m_deviceContext->PSSetSamplers(2, 1, m_shadowSampler.GetAddressOf());   // Slot 2 now

    m_deviceContext->IASetInputLayout(m_inputLayout.Get());
    m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
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

        m_deviceContext->VSSetConstantBuffers(0, 1, m_vsConstantBuffer.GetAddressOf());

        pGameObject->Draw(m_deviceContext.Get(), m_psMaterialConstantBuffer.Get());
    }
}