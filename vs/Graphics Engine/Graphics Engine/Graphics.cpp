#include "Graphics.h"
#include <vector>
#include <string>

// Link necessary libraries for DirectX
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// --- Embed Shaders ---
const char* vertexShaderSource = R"(
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
        output.pos = float4(input.pos, 1.0f);
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


Graphics::Graphics() = default;

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

    // Create Device, Device Context, and Swap Chain
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createDeviceFlags, featureLevels, 1,
        D3D11_SDK_VERSION, &sd, &m_swapChain,
        &m_device, nullptr, &m_deviceContext
    );
    ThrowIfFailed(hr);

    // Create Render Target View from the Swap Chain's back buffer
    Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer;
    ThrowIfFailed(m_swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer));
    ThrowIfFailed(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView));

    // Bind the Render Target View to the output merger stage
    m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

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
}

void Graphics::InitPipeline()
{
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    // Compile Vertex Shader from memory
    HRESULT hr = D3DCompile(vertexShaderSource, strlen(vertexShaderSource), "VertexShader", nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertexShaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            char* errorMessage = (char*)errorBlob->GetBufferPointer();
            throw std::runtime_error(std::string("Vertex Shader Compilation Error: ") + errorMessage);
        }
        else
        {
            ThrowIfFailed(hr);
        }
    }
    ThrowIfFailed(m_device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &m_vertexShader));

    // Define Input Layout
    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ThrowIfFailed(m_device->CreateInputLayout(inputLayoutDesc, ARRAYSIZE(inputLayoutDesc), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &m_inputLayout));

    // Compile Pixel Shader from memory
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
    errorBlob = nullptr;
    hr = D3DCompile(pixelShaderSource, strlen(pixelShaderSource), "PixelShader", nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixelShaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            char* errorMessage = (char*)errorBlob->GetBufferPointer();
            throw std::runtime_error(std::string("Pixel Shader Compilation Error: ") + errorMessage);
        }
        else
        {
            ThrowIfFailed(hr);
        }
    }
    ThrowIfFailed(m_device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &m_pixelShader));

    // Create Vertex Buffer
    Vertex vertices[] =
    {
        { DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = vertices;

    ThrowIfFailed(m_device->CreateBuffer(&bd, &sd, &m_vertexBuffer));
}


void Graphics::RenderFrame()
{
    // Clear the back buffer
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);

    // Set pipeline state
    m_deviceContext->IASetInputLayout(m_inputLayout.Get());
    m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // Bind vertex buffer
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

    // Set primitive topology
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw the triangle
    m_deviceContext->Draw(3, 0);

    // Present the frame
    ThrowIfFailed(m_swapChain->Present(1, 0));
}
