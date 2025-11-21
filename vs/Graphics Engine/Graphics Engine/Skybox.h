#pragma once
#include "EnginePCH.h"
#include <memory>
#include <string>
#include <DirectXMath.h>
#include <wrl/client.h>

class Mesh;
class VertexShader;
class PixelShader;
class Camera;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;
struct ID3D11Buffer;


class Skybox
{
public:
    Skybox();
    ~Skybox();

    Skybox(const Skybox&) = delete;
    Skybox& operator=(const Skybox&) = delete;

    void Init(ID3D11Device* device, ID3D11DeviceContext* context, const std::wstring& textureFilename);
    void Draw(ID3D11DeviceContext* context, const Camera& camera, const DirectX::XMMATRIX& projectionMatrix);

private:
    std::unique_ptr<Mesh> m_mesh;
    std::unique_ptr<VertexShader> m_vs;
    std::unique_ptr<PixelShader> m_ps;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureSRV;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dsState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsState;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
};
