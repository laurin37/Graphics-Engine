#include "include/EnginePCH.h"
#include "include/Shader.h"
#include "include/Graphics.h" 
#include <d3dcompiler.h>

void CompileShader(const std::wstring& filename, const std::string& entryPoint, const std::string& profile, ID3DBlob** blob)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(filename.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.c_str(), profile.c_str(), flags, 0, &shaderBlob, &errorBlob);

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            LOG_ERROR(std::string("Shader compilation failed:\n") + 
                     (char*)errorBlob->GetBufferPointer());
            throw std::runtime_error("Shader compilation failed.");
        }
        else
        {
            // This will be used if the file is not found, for example
            ThrowIfFailed(hr);
        }
    }

    *blob = shaderBlob.Detach();
}

void VertexShader::Init(ID3D11Device* device, const std::wstring& filename, const std::string& entryPoint, const D3D11_INPUT_ELEMENT_DESC* inputElementDesc, UINT numElements)
{
    CompileShader(filename, entryPoint, "vs_5_0", &m_shaderBlob);

    ThrowIfFailed(device->CreateVertexShader(m_shaderBlob->GetBufferPointer(), m_shaderBlob->GetBufferSize(), nullptr, &m_shader));

    if (inputElementDesc != nullptr)
    {
        ThrowIfFailed(device->CreateInputLayout(inputElementDesc, numElements, m_shaderBlob->GetBufferPointer(), m_shaderBlob->GetBufferSize(), &m_inputLayout));
    }
}

void VertexShader::Bind(ID3D11DeviceContext* context)
{
    context->VSSetShader(m_shader.Get(), nullptr, 0);
    if (m_inputLayout)
    {
        context->IASetInputLayout(m_inputLayout.Get());
    }
}

void PixelShader::Init(ID3D11Device* device, const std::wstring& filename, const std::string& entryPoint)
{
    CompileShader(filename, entryPoint, "ps_5_0", &m_shaderBlob);
    ThrowIfFailed(device->CreatePixelShader(m_shaderBlob->GetBufferPointer(), m_shaderBlob->GetBufferSize(), nullptr, &m_shader));
}

void PixelShader::Bind(ID3D11DeviceContext* context)
{
    context->PSSetShader(m_shader.Get(), nullptr, 0);
}
