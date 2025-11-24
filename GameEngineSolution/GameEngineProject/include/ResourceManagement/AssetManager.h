#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <memory>
#include <map>

#include "../Utils/EnginePCH.h"
#include "../Renderer/Mesh.h"

// Forward declaration
class Graphics;

class AssetManager
{
public:
    AssetManager(Graphics* graphics);
    ~AssetManager();

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // Asset Loading
    std::shared_ptr<Mesh> LoadMesh(const std::string& filePath);
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadTexture(const std::wstring& filePath);
    
    // Asset Retrieval
    std::shared_ptr<Mesh> GetMesh(const std::string& filePath);
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTexture(const std::wstring& filePath);
    std::shared_ptr<Mesh> GetDebugCube();

private:
    Graphics* m_graphics; // Raw pointer, lifetime managed by Game class

    std::map<std::string, std::shared_ptr<Mesh>> m_meshes;
    std::map<std::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;
};
