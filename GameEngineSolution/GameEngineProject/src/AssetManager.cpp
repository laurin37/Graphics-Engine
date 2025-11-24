#include "include/EnginePCH.h"
#include "include/AssetManager.h"
#include "include/Graphics.h"
#include "include/ModelLoader.h"
#include "include/TextureLoader.h"
#include "include/Mesh.h"

AssetManager::AssetManager(Graphics* graphics)
    : m_graphics(graphics)
{
    if (!m_graphics)
    {
        throw std::runtime_error("AssetManager requires a valid Graphics pointer!");
    }
}

AssetManager::~AssetManager() = default;

std::shared_ptr<Mesh> AssetManager::LoadMesh(const std::string& filePath)
{
    auto it = m_meshes.find(filePath);
    if (it != m_meshes.end())
    {
        return it->second;
    }

    auto mesh = ModelLoader::Load(m_graphics->GetDevice().Get(), filePath);
    if (mesh)
    {
        // ModelLoader returns unique_ptr, move it into the map
        m_meshes[filePath] = std::move(mesh);
        return m_meshes[filePath];
    }

    throw std::runtime_error("Failed to load mesh: " + filePath);
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> AssetManager::LoadTexture(const std::wstring& filePath)
{
    auto it = m_textures.find(filePath);
    if (it != m_textures.end())
    {
        return it->second;
    }

    auto texture = TextureLoader::Load(m_graphics->GetDevice().Get(), m_graphics->GetContext().Get(), filePath);
    if (texture)
    {
        m_textures[filePath] = texture;
        return texture;
    }
    
    throw std::runtime_error("Failed to load texture.");
}

std::shared_ptr<Mesh> AssetManager::GetMesh(const std::string& filePath)
{
    auto it = m_meshes.find(filePath);
    if (it != m_meshes.end())
    {
        return it->second;
    }
    throw std::runtime_error("Mesh not found: " + filePath);
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> AssetManager::GetTexture(const std::wstring& filePath)
{
    auto it = m_textures.find(filePath);
    if (it != m_textures.end())
    {
        return it->second;
    }
    throw std::runtime_error("Texture not found.");
}

std::shared_ptr<Mesh> AssetManager::GetDebugCube()
{
    const std::string debugCubeKey = "__debug_cube__";
    
    // Check if it's already created
    auto it = m_meshes.find(debugCubeKey);
    if (it != m_meshes.end())
    {
        return it->second;
    }

    std::vector<Vertex> vertices = {
        // Position only, other attributes are not needed for this debug mesh
        { { -0.5f, -0.5f, -0.5f }, {0,0}, {0,0,0}, {0,0,0} },
        { {  0.5f, -0.5f, -0.5f }, {0,0}, {0,0,0}, {0,0,0} },
        { {  0.5f,  0.5f, -0.5f }, {0,0}, {0,0,0}, {0,0,0} },
        { { -0.5f,  0.5f, -0.5f }, {0,0}, {0,0,0}, {0,0,0} },
        { { -0.5f, -0.5f,  0.5f }, {0,0}, {0,0,0}, {0,0,0} },
        { {  0.5f, -0.5f,  0.5f }, {0,0}, {0,0,0}, {0,0,0} },
        { {  0.5f,  0.5f,  0.5f }, {0,0}, {0,0,0}, {0,0,0} },
        { { -0.5f,  0.5f,  0.5f }, {0,0}, {0,0,0}, {0,0,0} }
    };

    std::vector<unsigned int> indices = {
        0, 1, 1, 2, 2, 3, 3, 0, // Front face
        4, 5, 5, 6, 6, 7, 7, 4, // Back face
        0, 4, 1, 5, 2, 6, 3, 7  // Connections
    };

    auto device = m_graphics->GetDevice().Get();
    auto mesh = std::make_shared<Mesh>(device, vertices, indices);

    m_meshes[debugCubeKey] = mesh;
    return mesh;
}
