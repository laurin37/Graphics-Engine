#pragma once

#include <string>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include "../Utils/EnginePCH.h"

void CompileShader(const std::wstring& filename, const std::string& entryPoint, const std::string& profile, ID3DBlob** blob);

class VertexShader
{
public:
    // If inputElementDesc is nullptr, no input layout will be created.
    void Init(ID3D11Device* device, const std::wstring& filename, const std::string& entryPoint, const D3D11_INPUT_ELEMENT_DESC* inputElementDesc, UINT numElements);
    void Bind(ID3D11DeviceContext* context);

    ID3D11InputLayout* GetInputLayout() const { return m_inputLayout.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_shader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3DBlob> m_shaderBlob;
};

class PixelShader
{
public:
    void Init(ID3D11Device* device, const std::wstring& filename, const std::string& entryPoint);
    void Bind(ID3D11DeviceContext* context);

private:
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_shader;
    Microsoft::WRL::ComPtr<ID3DBlob> m_shaderBlob;
};
