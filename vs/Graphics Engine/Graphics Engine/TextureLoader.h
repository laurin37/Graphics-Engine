#pragma once

#include "EnginePCH.h"
#include <string>
#include <wrl/client.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;

class TextureLoader
{
public:
    static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Load(ID3D11Device* device, ID3D11DeviceContext* context, const std::wstring& filename);
};