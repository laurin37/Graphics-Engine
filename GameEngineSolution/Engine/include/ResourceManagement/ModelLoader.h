#pragma once

#include <memory>
#include <string>
#include <vector>
#include <DirectXMath.h>

struct ID3D11Device;
class Mesh;

class ModelLoader
{
public:
    static std::unique_ptr<Mesh> Load(ID3D11Device* device, const std::string& filePath, float scale = 1.0f);
};
