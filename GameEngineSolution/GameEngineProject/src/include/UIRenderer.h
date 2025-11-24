#pragma once

#include "EnginePCH.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <string>

// Forward Declarations
class Graphics;
class SimpleFont;

// --- UI Structures ---
struct SpriteVertex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uv;
    DirectX::XMFLOAT4 color;
};

struct CB_VS_UI
{
    DirectX::XMFLOAT2 screenSize;
    DirectX::XMFLOAT2 padding;
};

class UIRenderer
{
public:
    UIRenderer(Graphics* graphics);
    ~UIRenderer() = default;

    UIRenderer(const UIRenderer&) = delete;
    UIRenderer& operator=(const UIRenderer&) = delete;

    void DrawSprite(const SpriteVertex* vertices, size_t count, ID3D11ShaderResourceView* texture);
    void DrawString(const SimpleFont& font, const std::string& text, float x, float y, float size, const float color[4]);

    void EnableUIState();
    void DisableUIState();

private:
    void Initialize();

    Graphics* m_graphics; // Raw pointer, lifetime managed by Game class

    // UI Objects
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_uiVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_uiPS;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_uiVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_uiConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_uiBlendState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_uiDepthStencilState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_uiRS;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_uiInputLayout;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_uiSamplerState;
};
