#pragma once

#include "JsonParser.h"
#include "../ECS/ComponentManager.h"
#include "../ECS/Components.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <DirectXMath.h>

// Forward declarations
class Mesh;
class Material;
class AssetManager;

// ==================================================================================
// SceneLoader Class
// ----------------------------------------------------------------------------------
// Static utility class for loading scenes from JSON files.
//
// JSON Structure:
// {
//   "resources": {
//     "meshes": { "meshName": "path/to/mesh.obj" },
//     "materials": { "matName": { "color": [r,g,b,a], "texture": "path/to/tex.png" } }
//   },
//   "entities": [
//     {
//       "components": {
//         "transform": { "position": [x,y,z], ... },
//         "render": { "mesh": "meshName", "material": "matName" },
//         ...
//       }
//     }
//   ]
// }
// ==================================================================================
class SceneLoader {
public:
    // Load a scene from JSON file and populate the ComponentManager
    static void LoadScene(
        const std::wstring& jsonPath,
        ECS::ComponentManager& componentManager,
        AssetManager* assetManager
    );

private:
    // Parse resources section (meshes, materials)
    static void ParseResources(
        const JsonValue& resources, 
        AssetManager* assetManager,
        std::unordered_map<std::string, Mesh*>& outMeshLookup,
        std::unordered_map<std::string, std::shared_ptr<Material>>& outMaterialLookup
    );
    // Component parsers (take JsonValue, return component structs)
    static ECS::TransformComponent ParseTransform(const JsonValue& j);
    static ECS::PhysicsComponent ParsePhysics(const JsonValue& j);
    static ECS::RenderComponent ParseRender(
        const JsonValue& j,
        const std::unordered_map<std::string, Mesh*>& meshLookup,
        const std::unordered_map<std::string, std::shared_ptr<Material>>& materialLookup
    );
    static ECS::ColliderComponent ParseCollider(const JsonValue& j, Mesh* mesh);
    static ECS::LightComponent ParseLight(const JsonValue& j);
    static ECS::RotateComponent ParseRotate(const JsonValue& j);
    static ECS::OrbitComponent ParseOrbit(const JsonValue& j);
    static ECS::PlayerControllerComponent ParsePlayerController(const JsonValue& j);
    static ECS::CameraComponent ParseCamera(const JsonValue& j);
    static ECS::HealthComponent ParseHealth(const JsonValue& j);
    static ECS::WeaponComponent ParseWeapon(const JsonValue& j);
    static ECS::ProjectileComponent ParseProjectile(const JsonValue& j);

    // Helper: Extract vec3/vec4 from JSON array [x, y, z]
    static DirectX::XMFLOAT3 ParseVec3(const JsonValue& arr, const DirectX::XMFLOAT3& defaultValue = {0.0f, 0.0f, 0.0f});
    static DirectX::XMFLOAT4 ParseVec4(const JsonValue& arr, const DirectX::XMFLOAT4& defaultValue = {0.0f, 0.0f, 0.0f, 0.0f});
    
    // Helper: Get field with default value
    template<typename T>
    static T GetFieldOrDefault(const JsonValue& obj, const std::string& key, T defaultValue);
};
