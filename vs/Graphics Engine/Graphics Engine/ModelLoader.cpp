#include "EnginePCH.h"
#include "ModelLoader.h"
#include "Mesh.h"
#include <fstream>
#include <sstream>
#include <map>
#include <tuple>
#include <windows.h> // For OutputDebugStringA

// A key to uniquely identify a vertex by its attribute indices
using VertexKey = std::tuple<int, int, int>;

// Processes one corner of a face, creating a new vertex if it's unique
// and returning the index for the index buffer.
static unsigned int process_face_corner(
    const std::string& corner_str,
    const std::vector<DirectX::XMFLOAT3>& temp_positions,
    const std::vector<DirectX::XMFLOAT2>& temp_uvs,
    const std::vector<DirectX::XMFLOAT3>& temp_normals,
    std::vector<Vertex>& finalVertices,
    std::map<VertexKey, unsigned int>& vertexIndexMap)
{
    std::stringstream corner_ss(corner_str);
    std::string p_str, t_str, n_str;

    long p_idx = 0, t_idx = 0, n_idx = 0;

    std::getline(corner_ss, p_str, '/');
    if (!p_str.empty()) p_idx = std::stol(p_str);

    if (std::getline(corner_ss, t_str, '/')) {
        if (!t_str.empty()) t_idx = std::stol(t_str);
    }
    if (std::getline(corner_ss, n_str, '/')) {
        if (!n_str.empty()) n_idx = std::stol(n_str);
    }

    // Handle negative indices (relative to the current list size)
    if (p_idx < 0) p_idx = temp_positions.size() + p_idx + 1;
    if (t_idx < 0) t_idx = temp_uvs.size() + t_idx + 1;
    if (n_idx < 0) n_idx = temp_normals.size() + n_idx + 1;

    // Safety check indices before access
    if (p_idx <= 0 || p_idx > temp_positions.size()) throw std::runtime_error("Invalid position index in OBJ file.");
    if (t_idx != 0 && (t_idx <= 0 || t_idx > temp_uvs.size())) throw std::runtime_error("Invalid texture coordinate index in OBJ file.");
    if (n_idx != 0 && (n_idx <= 0 || n_idx > temp_normals.size())) throw std::runtime_error("Invalid normal index in OBJ file.");

    VertexKey key = std::make_tuple(p_idx, t_idx, n_idx);
    auto it = vertexIndexMap.find(key);

    if (it != vertexIndexMap.end())
    {
        return it->second;
    }

    // Vertex not found, create it
    Vertex v;
    v.pos = temp_positions[p_idx - 1];
    v.uv = (t_idx > 0) ? temp_uvs[t_idx - 1] : DirectX::XMFLOAT2(0.0f, 0.0f);
    v.normal = (n_idx > 0) ? temp_normals[n_idx - 1] : DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f); // Default normal up

    // FIX: Initialize tangent to zero so accumulation works correctly
    v.tangent = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

    finalVertices.push_back(v);
    unsigned int newIndex = static_cast<unsigned int>(finalVertices.size() - 1);
    vertexIndexMap[key] = newIndex;
    return newIndex;
}


std::unique_ptr<Mesh> ModelLoader::Load(ID3D11Device* device, const std::string& filePath, float scale)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open model file: " + filePath);
    }

    std::vector<DirectX::XMFLOAT3> temp_positions;
    std::vector<DirectX::XMFLOAT2> temp_uvs;
    std::vector<DirectX::XMFLOAT3> temp_normals;

    std::vector<Vertex> finalVertices;
    std::vector<unsigned int> finalIndices;
    std::map<VertexKey, unsigned int> vertexIndexMap;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.length() < 2) continue;
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v")
        {
            DirectX::XMFLOAT3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            pos.x *= scale;
            pos.y *= scale;
            pos.z *= scale;
            temp_positions.push_back(pos);
        }
        else if (prefix == "vt")
        {
            DirectX::XMFLOAT2 uv;
            ss >> uv.x >> uv.y;
            uv.y = 1.0f - uv.y; // Invert V for DirectX
            temp_uvs.push_back(uv);
        }
        else if (prefix == "vn")
        {
            DirectX::XMFLOAT3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (prefix == "f")
        {
            std::string corner_str;
            std::vector<unsigned int> face_indices;
            while (ss >> corner_str)
            {
                face_indices.push_back(process_face_corner(
                    corner_str, temp_positions, temp_uvs, temp_normals, finalVertices, vertexIndexMap
                ));
            }

            // Triangulate the face (fan triangulation)
            if (face_indices.size() >= 3)
            {
                for (size_t i = 1; i < face_indices.size() - 1; ++i)
                {
                    finalIndices.push_back(face_indices[0]);
                    finalIndices.push_back(face_indices[i]);
                    finalIndices.push_back(face_indices[i + 1]);
                }
            }
        }
    }

    // --- Tangent Calculation ---
    // We need to calculate tangents for normal mapping.
    // We'll do this after all vertices and indices are loaded.
    for (size_t i = 0; i < finalIndices.size(); i += 3)
    {
        // Get the vertices of the triangle
        Vertex& v0 = finalVertices[finalIndices[i]];
        Vertex& v1 = finalVertices[finalIndices[i + 1]];
        Vertex& v2 = finalVertices[finalIndices[i + 2]];

        // Get the edges of the triangle
        DirectX::XMFLOAT3 edge1(v1.pos.x - v0.pos.x, v1.pos.y - v0.pos.y, v1.pos.z - v0.pos.z);
        DirectX::XMFLOAT3 edge2(v2.pos.x - v0.pos.x, v2.pos.y - v0.pos.y, v2.pos.z - v0.pos.z);

        // Get the delta UVs
        DirectX::XMFLOAT2 deltaUV1(v1.uv.x - v0.uv.x, v1.uv.y - v0.uv.y);
        DirectX::XMFLOAT2 deltaUV2(v2.uv.x - v0.uv.x, v2.uv.y - v0.uv.y);

        // Calculate the tangent
        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        DirectX::XMFLOAT3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        // Accumulate the tangent for each vertex
        v0.tangent.x += tangent.x; v0.tangent.y += tangent.y; v0.tangent.z += tangent.z;
        v1.tangent.x += tangent.x; v1.tangent.y += tangent.y; v1.tangent.z += tangent.z;
        v2.tangent.x += tangent.x; v2.tangent.y += tangent.y; v2.tangent.z += tangent.z;
    }

    // Normalize the tangents
    for (auto& v : finalVertices)
    {
        DirectX::XMVECTOR tangent = DirectX::XMLoadFloat3(&v.tangent);
        DirectX::XMVECTOR normal = DirectX::XMLoadFloat3(&v.normal);

        // Gram-Schmidt orthogonalize and normalize the tangent
        tangent = DirectX::XMVector3Normalize(
            DirectX::XMVectorSubtract(tangent, DirectX::XMVectorMultiply(normal, DirectX::XMVector3Dot(normal, tangent)))
        );

        DirectX::XMStoreFloat3(&v.tangent, tangent);
    }


    std::string debug_msg = "Loaded model: " + filePath + ", Vertices: " + std::to_string(finalVertices.size()) + ", Indices: " + std::to_string(finalIndices.size()) + "\n";
    OutputDebugStringA(debug_msg.c_str());

    if (finalVertices.empty() || finalIndices.empty())
    {
        throw std::runtime_error("Model has no valid geometry: " + filePath);
    }

    return std::make_unique<Mesh>(device, finalVertices, finalIndices);
}