#include "Graphics.h"
#include <vector>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

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
        float3 normal : NORMAL;
    };

    struct PS_INPUT
    {
        float4 pos : SV_POSITION;
        float2 uv : TEXCOORD;
        float3 normal : NORMAL;
        float3 worldPos : WORLD_POS;
    };

    PS_INPUT main(VS_INPUT input)
    {
        PS_INPUT output;
        float4 pos = float4(input.pos, 1.0f);
        
        // Transform position to world space for the pixel shader
        output.worldPos = mul(pos, worldMatrix).xyz;

        // Transform position to clip space
        pos = mul(pos, worldMatrix);
        pos = mul(pos, viewMatrix);
        pos = mul(pos, projectionMatrix);
        output.pos = pos;
        
        // Pass normal and UV to pixel shader
        output.normal = normalize(mul(input.normal, (float3x3)worldMatrix));
        output.uv = input.uv;
        return output;
    }
)";

const char* pixelShaderSource = R"(
    Texture2D proceduralTexture : register(t0);
    SamplerState pointSampler : register(s0);

    cbuffer LightBuffer : register(b0)
    {
        float4 lightDir;
        float4 lightColor;
        float4 cameraPos;
        float specularIntensity;
        float specularPower;
    }

    struct PS_INPUT
    {
        float4 pos : SV_POSITION;
        float2 uv : TEXCOORD;
        float3 normal : NORMAL;
        float3 worldPos : WORLD_POS;
    };

    float4 main(PS_INPUT input) : SV_TARGET
    {
        float4 texColor = proceduralTexture.Sample(pointSampler, input.uv);
        
        // Ambient
        float ambient = 0.1f;
        
        // Diffuse
        float diffuse = saturate(dot(input.normal, -lightDir.xyz));
        float3 diffuseColor = texColor.rgb * (diffuse * lightColor.rgb + ambient);

        // Specular (Blinn-Phong)
        float3 viewDir = normalize(cameraPos.xyz - input.worldPos);
        float3 halfVector = normalize(-lightDir.xyz + viewDir);
        float spec = pow(saturate(dot(input.normal, halfVector)), specularPower);
        float3 specColor = specularIntensity * spec * lightColor.rgb;

        // Final color
        float3 finalColor = diffuseColor + specColor;
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

    m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<float>(width);
    vp.Height = static_cast<float>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    m_deviceContext->RSSetViewports(1, &vp);

    InitPipeline();

    // Initialize camera
    m_camera = std::make_unique<Camera>();
    m_camera->SetPosition(0.0f, 0.0f, -2.5f);

    // Setup projection matrix
    m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV4, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f
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
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ThrowIfFailed(m_device->CreateInputLayout(inputLayoutDesc, ARRAYSIZE(inputLayoutDesc), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &m_inputLayout));

    hr = D3DCompile(pixelShaderSource, strlen(pixelShaderSource), "PixelShader", nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixelShaderBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) { throw std::runtime_error(std::string("PS Error: ") + (char*)errorBlob->GetBufferPointer()); } else { ThrowIfFailed(hr); } }
    ThrowIfFailed(m_device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &m_pixelShader));

    // Create cube mesh data
    std::vector<Vertex> vertices = {
        { {-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f} }, { {-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} }, { { 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f} }, { { 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} },
        { { 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },  { { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },  { {-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },  { {-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
        { {-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },  { {-0.5f, 0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },  { { 0.5f, 0.5f,  0.5f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },  { { 0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },
        { { 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f} }, { { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f} }, { {-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f} }, { {-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} },
        { {-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} }, { {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} }, { {-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} }, { {-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} },
        { { 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} },  { { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },  { { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },  { { 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} }
    };
    std::vector<unsigned int> indices = { 0,1,2, 0,2,3, 4,5,6, 4,6,7, 8,9,10, 8,10,11, 12,13,14, 12,14,15, 16,17,18, 16,18,19, 20,21,22, 20,22,23 };
    m_cubeMesh = std::make_unique<Mesh>(m_device.Get(), vertices, indices);

    // Create constant buffers
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(CB_VS_vertexshader);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ThrowIfFailed(m_device->CreateBuffer(&cbd, nullptr, &m_vsConstantBuffer));

    cbd.ByteWidth = sizeof(CB_PS_light);
    ThrowIfFailed(m_device->CreateBuffer(&cbd, nullptr, &m_psConstantBuffer));

    // Create texture and sampler
    const int texWidth = 64, texHeight = 64;
    std::vector<uint32_t> texData(texWidth * texHeight);
    for (int y = 0; y < texHeight; ++y) for (int x = 0; x < texWidth; ++x) texData[y * texWidth + x] = ((x / 8 % 2) == (y / 8 % 2)) ? 0xFFFFFFFF : 0xFF000000;
    
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
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ThrowIfFailed(m_device->CreateSamplerState(&sampDesc, &m_samplerState));
}

Camera* Graphics::GetCamera()
{
    return m_camera.get();
}

void Graphics::RenderFrame()
{
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Set world matrix to identity
    m_worldMatrix = DirectX::XMMatrixIdentity();

    // Get view matrix from camera
    DirectX::XMMATRIX viewMatrix = m_camera->GetViewMatrix();

    // Update VS constant buffer
    CB_VS_vertexshader vs_cb;
    DirectX::XMStoreFloat4x4(&vs_cb.worldMatrix, DirectX::XMMatrixTranspose(m_worldMatrix));
    DirectX::XMStoreFloat4x4(&vs_cb.viewMatrix, DirectX::XMMatrixTranspose(viewMatrix));
    DirectX::XMStoreFloat4x4(&vs_cb.projectionMatrix, DirectX::XMMatrixTranspose(m_projectionMatrix));
    m_deviceContext->UpdateSubresource(m_vsConstantBuffer.Get(), 0, nullptr, &vs_cb, 0, 0);

    // Update PS constant buffer
    CB_PS_light ps_cb;
    DirectX::XMStoreFloat4(&ps_cb.lightDir, DirectX::XMVector3Normalize(DirectX::XMVectorSet(0.5f, -0.5f, 1.0f, 0.0f)));
    ps_cb.lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMStoreFloat4(&ps_cb.cameraPos, m_camera->GetPosition());
    ps_cb.specularIntensity = 1.0f;
    ps_cb.specularPower = 32.0f;
    m_deviceContext->UpdateSubresource(m_psConstantBuffer.Get(), 0, nullptr, &ps_cb, 0, 0);

    // Bind constant buffers
    m_deviceContext->VSSetConstantBuffers(0, 1, m_vsConstantBuffer.GetAddressOf());
    m_deviceContext->PSSetConstantBuffers(0, 1, m_psConstantBuffer.GetAddressOf());

    // Bind textures and samplers
    m_deviceContext->PSSetShaderResources(0, 1, m_textureView.GetAddressOf());
    m_deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
    
    // Set common pipeline state
    m_deviceContext->IASetInputLayout(m_inputLayout.Get());
    m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw the mesh
    m_cubeMesh->Draw(m_deviceContext.Get());

    // Present the frame
    ThrowIfFailed(m_swapChain->Present(1, 0));
}
