#pragma once

#include <string>
#include <wrl/client.h>

#include "../Utils/EnginePCH.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;

class TextureLoader
{
public:
    static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Load(ID3D11Device* device, ID3D11DeviceContext* context, const std::wstring& filename);

    // Creates a simple generated font texture in memory (No file needed)
    static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateDebugFont(ID3D11Device* device, ID3D11DeviceContext* context);
};