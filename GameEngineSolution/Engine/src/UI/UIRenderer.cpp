#include "../../include/Utils/EnginePCH.h"
#include "../../include/UI/UIRenderer.h"
#include "../../include/Renderer/Graphics.h"
#include "../../include/UI/SimpleFont.h"

UIRenderer::UIRenderer(Graphics* graphics)
    : m_graphics(graphics)
{
    if (!m_graphics)
    {
        throw std::runtime_error("UIRenderer requires a valid Graphics pointer!");
    }
    Initialize();
}

void UIRenderer::Initialize()
{
    auto device = m_graphics->GetDevice();

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> uiVSBlob, uiPSBlob, errorBlob;

    // Compile Vertex Shader
    HRESULT hr = D3DCompileFromFile(L"../Assets/Shaders/UIVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", flags, 0, &uiVSBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            std::string errorMessage = "UI Vertex Shader compilation failed: ";
            errorMessage += (char*)errorBlob->GetBufferPointer();
            throw std::runtime_error(errorMessage);
        }
        else
        {
            ThrowIfFailed(hr);
        }
    }
    ThrowIfFailed(device->CreateVertexShader(uiVSBlob->GetBufferPointer(), uiVSBlob->GetBufferSize(), nullptr, &m_uiVS));

    // Compile Pixel Shader
    hr = D3DCompileFromFile(L"../Assets/Shaders/UIPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", flags, 0, &uiPSBlob, &errorBlob);
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
    ThrowIfFailed(device->CreatePixelShader(uiPSBlob->GetBufferPointer(), uiPSBlob->GetBufferSize(), nullptr, &m_uiPS));

    D3D11_INPUT_ELEMENT_DESC uiInputDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ThrowIfFailed(device->CreateInputLayout(uiInputDesc, ARRAYSIZE(uiInputDesc), uiVSBlob->GetBufferPointer(), uiVSBlob->GetBufferSize(), &m_uiInputLayout));

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.ByteWidth = sizeof(CB_VS_UI);
    ThrowIfFailed(device->CreateBuffer(&cbd, nullptr, &m_uiConstantBuffer));

    D3D11_BUFFER_DESC uiVBD = {};
    uiVBD.Usage = D3D11_USAGE_DYNAMIC;
    uiVBD.ByteWidth = sizeof(SpriteVertex) * MAX_BATCH_SIZE * 6; // Use constant
    uiVBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    uiVBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ThrowIfFailed(device->CreateBuffer(&uiVBD, nullptr, &m_uiVertexBuffer));

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    ThrowIfFailed(device->CreateBlendState(&blendDesc, &m_uiBlendState));

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = FALSE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    ThrowIfFailed(device->CreateDepthStencilState(&dsDesc, &m_uiDepthStencilState));

    D3D11_RASTERIZER_DESC uiRSDesc = {};
    uiRSDesc.FillMode = D3D11_FILL_SOLID;
    uiRSDesc.CullMode = D3D11_CULL_NONE;
    uiRSDesc.DepthClipEnable = true;
    ThrowIfFailed(device->CreateRasterizerState(&uiRSDesc, &m_uiRS));

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ThrowIfFailed(device->CreateSamplerState(&sampDesc, &m_uiSamplerState));
}

void UIRenderer::EnableUIState()
{
    auto context = m_graphics->GetContext();

    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context->OMSetBlendState(m_uiBlendState.Get(), blendFactor, 0xffffffff);
    context->OMSetDepthStencilState(m_uiDepthStencilState.Get(), 0);
    context->RSSetState(m_uiRS.Get());

    CB_VS_UI cbUI;
    cbUI.screenSize = DirectX::XMFLOAT2(m_graphics->GetScreenWidth(), m_graphics->GetScreenHeight());
    cbUI.padding = DirectX::XMFLOAT2(0, 0);
    context->UpdateSubresource(m_uiConstantBuffer.Get(), 0, nullptr, &cbUI, 0, 0);
    context->VSSetConstantBuffers(0, 1, m_uiConstantBuffer.GetAddressOf());
    
    BeginBatch();
}

void UIRenderer::DisableUIState()
{
    Flush(); // Draw any remaining sprites
    auto context = m_graphics->GetContext();
    context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    context->OMSetDepthStencilState(nullptr, 0);
    context->RSSetState(nullptr);
}

void UIRenderer::BeginBatch()
{
    m_currentBatch.vertices.clear();
    m_currentBatch.texture = nullptr;
}

void UIRenderer::Flush()
{
    if (m_currentBatch.vertices.empty() || !m_currentBatch.texture)
    {
        return;
    }

    auto context = m_graphics->GetContext();

    // Resize buffer if needed (simple dynamic buffer strategy)
    // For now, we assume MAX_BATCH_SIZE is enough or we flush when full
    
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = context->Map(m_uiVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr)) return;

    size_t vertexCount = m_currentBatch.vertices.size();
    memcpy(mappedResource.pData, m_currentBatch.vertices.data(), sizeof(SpriteVertex) * vertexCount);
    context->Unmap(m_uiVertexBuffer.Get(), 0);

    UINT stride = sizeof(SpriteVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_uiVertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetInputLayout(m_uiInputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(m_uiVS.Get(), nullptr, 0);
    context->PSSetShader(m_uiPS.Get(), nullptr, 0);
    context->PSSetShaderResources(0, 1, &m_currentBatch.texture);
    context->PSSetSamplers(0, 1, m_uiSamplerState.GetAddressOf());

    context->Draw(static_cast<UINT>(vertexCount), 0);

    m_currentBatch.vertices.clear();
    // Keep texture for next batch potentially
}

void UIRenderer::DrawSprite(const SpriteVertex* vertices, size_t count, ID3D11ShaderResourceView* texture)
{
    if (count == 0) return;

    // If texture changes, flush
    if (m_currentBatch.texture != texture)
    {
        Flush();
        m_currentBatch.texture = texture;
    }

    // If batch is full, flush
    if (m_currentBatch.vertices.size() + count > MAX_BATCH_SIZE * 6)
    {
        Flush();
        m_currentBatch.texture = texture; // Re-set texture after flush
    }

    // Append vertices
    m_currentBatch.vertices.insert(m_currentBatch.vertices.end(), vertices, vertices + count);
}

void UIRenderer::DrawString(const SimpleFont& font, const std::string& text, float x, float y, float size, const float color[4])
{
    auto vertices = font.GenerateVerticesForString(text, x, y, size, color);
    if (vertices.empty())
    {
        return;
    }

    DrawSprite(vertices.data(), vertices.size(), font.GetTexture());
}
