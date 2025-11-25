#include "../../include/ResourceManagement/SceneLoader.h"
#include "../../include/Renderer/Mesh.h"
#include "../../include/Renderer/Material.h"
#include "../../include/Physics/Collision.h"
#include <algorithm>
#include <format>

// Helper to calculate AABB from mesh (same as in Scene.cpp)
static ECS::ColliderComponent CalculateCollider(const Mesh* mesh) {
    ECS::ColliderComponent collider;
    if (!mesh || mesh->GetVertexCount() == 0) return collider;

    DirectX::XMFLOAT3 minPos = { FLT_MAX, FLT_MAX, FLT_MAX };
    DirectX::XMFLOAT3 maxPos = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (const auto& v : mesh->GetVertices()) {
        minPos.x = (std::min)(minPos.x, v.pos.x);
        minPos.y = (std::min)(minPos.y, v.pos.y);
        minPos.z = (std::min)(minPos.z, v.pos.z);

        maxPos.x = (std::max)(maxPos.x, v.pos.x);
        maxPos.y = (std::max)(maxPos.y, v.pos.y);
        maxPos.z = (std::max)(maxPos.z, v.pos.z);
    }

    // Calculate center and extents (half-size)
    collider.localAABB.center = {
        (minPos.x + maxPos.x) * 0.5f,
        (minPos.y + maxPos.y) * 0.5f,
        (minPos.z + maxPos.z) * 0.5f
    };

    collider.localAABB.extents = {
        (maxPos.x - minPos.x) * 0.5f,
        (maxPos.y - minPos.y) * 0.5f,
        (maxPos.z - minPos.z) * 0.5f
    };
    
    collider.enabled = true;
    return collider;
}

// ========================================
// SceneLoader Implementation
// ========================================

void SceneLoader::LoadScene(
    const std::wstring& jsonPath,
    ECS::ComponentManager& componentManager,
    const std::unordered_map<std::string, Mesh*>& meshLookup,
    const std::unordered_map<std::string, std::shared_ptr<Material>>& materialLookup
) {
    // Parse JSON file
    JsonValue root = JsonParser::ParseFile(jsonPath);
    
    if (!root.IsObject()) {
        throw std::runtime_error("Scene JSON root must be an object");
    }
    
    if (!root.HasField("entities")) {
        throw std::runtime_error("Scene JSON must have 'entities' array");
    }
    
    const JsonValue& entitiesArray = root.GetField("entities");
    if (!entitiesArray.IsArray()) {
        throw std::runtime_error("'entities' must be an array");
    }
    
    // Create each entity
    for (size_t i = 0; i < entitiesArray.ArraySize(); ++i) {
        const JsonValue& entityDef = entitiesArray[i];
        
        if (!entityDef.IsObject()) {
            throw std::runtime_error(std::format("Entity {} must be an object", i));
        }
        
        // Create entity
        ECS::Entity entity = componentManager.CreateEntity();
        
        // Check for components object
        if (!entityDef.HasField("components")) {
            continue; // Entity with no components (valid but useless)
        }
        
        const JsonValue& components = entityDef.GetField("components");
        if (!components.IsObject()) {
            throw std::runtime_error("'components' must be an object");
        }
        
        // Track mesh for collider auto-generation
        Mesh* entityMesh = nullptr;
        
        // Parse Transform
        if (components.HasField("transform")) {
            ECS::TransformComponent transform = ParseTransform(components.GetField("transform"));
            componentManager.AddComponent<ECS::TransformComponent>(entity, transform);
        }
        
        // Parse Render (must come before collider for auto-generation)
        if (components.HasField("render")) {
            ECS::RenderComponent render = ParseRender(components.GetField("render"), meshLookup, materialLookup);
            componentManager.AddComponent<ECS::RenderComponent>(entity, render);
            entityMesh = render.mesh;
        }
        
        // Parse Physics
        if (components.HasField("physics")) {
            ECS::PhysicsComponent physics = ParsePhysics(components.GetField("physics"));
            componentManager.AddComponent<ECS::PhysicsComponent>(entity, physics);
        }
        
        // Parse Collider
        if (components.HasField("collider")) {
            ECS::ColliderComponent collider = ParseCollider(components.GetField("collider"), entityMesh);
            componentManager.AddComponent<ECS::ColliderComponent>(entity, collider);
        }
        
        // Parse Light
        if (components.HasField("light")) {
            ECS::LightComponent light = ParseLight(components.GetField("light"));
            componentManager.AddComponent<ECS::LightComponent>(entity, light);
        }
        
        // Parse Rotate
        if (components.HasField("rotate")) {
            ECS::RotateComponent rotate = ParseRotate(components.GetField("rotate"));
            componentManager.AddComponent<ECS::RotateComponent>(entity, rotate);
        }
        
        // Parse Orbit
        if (components.HasField("orbit")) {
            ECS::OrbitComponent orbit = ParseOrbit(components.GetField("orbit"));
            componentManager.AddComponent<ECS::OrbitComponent>(entity, orbit);
        }
        
        // Parse PlayerController
        if (components.HasField("playerController")) {
            ECS::PlayerControllerComponent controller = ParsePlayerController(components.GetField("playerController"));
            componentManager.AddComponent<ECS::PlayerControllerComponent>(entity, controller);
        }

        // Parse Camera
        if (components.HasField("camera")) {
            ECS::CameraComponent camera = ParseCamera(components.GetField("camera"));
            componentManager.AddComponent<ECS::CameraComponent>(entity, camera);
        }
    }
}

// ========================================
// Component Parsing Methods
// ========================================

ECS::CameraComponent SceneLoader::ParseCamera(const JsonValue& j) {
    ECS::CameraComponent camera;
    
    if (j.HasField("fov")) {
        camera.fov = static_cast<float>(j.GetField("fov").AsNumber());
    }
    
    if (j.HasField("aspectRatio")) {
        camera.aspectRatio = static_cast<float>(j.GetField("aspectRatio").AsNumber());
    }
    
    if (j.HasField("nearPlane")) {
        camera.nearPlane = static_cast<float>(j.GetField("nearPlane").AsNumber());
    }
    
    if (j.HasField("farPlane")) {
        camera.farPlane = static_cast<float>(j.GetField("farPlane").AsNumber());
    }
    
    if (j.HasField("isActive")) {
        camera.isActive = j.GetField("isActive").AsBool();
    }
    
    if (j.HasField("offset")) {
        camera.positionOffset = ParseVec3(j.GetField("offset"), {0.0f, 0.0f, 0.0f});
    }
    
    return camera;
}

ECS::TransformComponent SceneLoader::ParseTransform(const JsonValue& j) {
    ECS::TransformComponent transform;
    
    if (j.HasField("position")) {
        transform.position = ParseVec3(j.GetField("position"), {0.0f, 0.0f, 0.0f});
    }
    
    if (j.HasField("rotation")) {
        transform.rotation = ParseVec3(j.GetField("rotation"), {0.0f, 0.0f, 0.0f});
    }
    
    if (j.HasField("scale")) {
        transform.scale = ParseVec3(j.GetField("scale"), {1.0f, 1.0f, 1.0f});
    }
    
    return transform;
}

ECS::PhysicsComponent SceneLoader::ParsePhysics(const JsonValue& j) {
    ECS::PhysicsComponent physics;
    
    if (j.HasField("velocity")) {
        physics.velocity = ParseVec3(j.GetField("velocity"), {0.0f, 0.0f, 0.0f});
    }
    
    if (j.HasField("acceleration")) {
        physics.acceleration = ParseVec3(j.GetField("acceleration"), {0.0f, 0.0f, 0.0f});
    }
    
    if (j.HasField("mass")) {
        physics.mass = static_cast<float>(j.GetField("mass").AsNumber());
    }
    
    if (j.HasField("drag")) {
        physics.drag = static_cast<float>(j.GetField("drag").AsNumber());
    }
    
    if (j.HasField("gravityAcceleration")) {
        physics.gravityAcceleration = static_cast<float>(j.GetField("gravityAcceleration").AsNumber());
    }
    
    if (j.HasField("maxFallSpeed")) {
        physics.maxFallSpeed = static_cast<float>(j.GetField("maxFallSpeed").AsNumber());
    }
    
    if (j.HasField("useGravity")) {
        physics.useGravity = j.GetField("useGravity").AsBool();
    }
    
    if (j.HasField("checkCollisions")) {
        physics.checkCollisions = j.GetField("checkCollisions").AsBool();
    }
    
    if (j.HasField("isGrounded")) {
        physics.isGrounded = j.GetField("isGrounded").AsBool();
    }
    
    return physics;
}

ECS::RenderComponent SceneLoader::ParseRender(
    const JsonValue& j,
    const std::unordered_map<std::string, Mesh*>& meshLookup,
    const std::unordered_map<std::string, std::shared_ptr<Material>>& materialLookup
) {
    ECS::RenderComponent render;
    
    // Parse mesh
    if (j.HasField("mesh")) {
        std::string meshName = j.GetField("mesh").AsString();
        auto it = meshLookup.find(meshName);
        if (it == meshLookup.end()) {
            throw std::runtime_error("Mesh not found: " + meshName);
        }
        render.mesh = it->second;
    }
    
    // Parse material
    if (j.HasField("material")) {
        std::string materialName = j.GetField("material").AsString();
        auto it = materialLookup.find(materialName);
        if (it == materialLookup.end()) {
            throw std::runtime_error("Material not found: " + materialName);
        }
        render.material = it->second;
    }
    
    return render;
}

ECS::ColliderComponent SceneLoader::ParseCollider(const JsonValue& j, Mesh* mesh) {
    ECS::ColliderComponent collider;
    
    // Check for auto-generation
    if (j.HasField("autoGenerate") && j.GetField("autoGenerate").AsBool()) {
        if (!mesh) {
            throw std::runtime_error("Cannot auto-generate collider: no mesh available");
        }
        return CalculateCollider(mesh);
    }
    
    // Manual collider definition
    if (j.HasField("center")) {
        collider.localAABB.center = ParseVec3(j.GetField("center"), {0.0f, 0.0f, 0.0f});
    }
    
    if (j.HasField("extents")) {
        collider.localAABB.extents = ParseVec3(j.GetField("extents"), {0.5f, 0.5f, 0.5f});
    }
    
    if (j.HasField("enabled")) {
        collider.enabled = j.GetField("enabled").AsBool();
    }
    
    return collider;
}

ECS::LightComponent SceneLoader::ParseLight(const JsonValue& j) {
    ECS::LightComponent light;
    
    if (j.HasField("color")) {
        light.color = ParseVec4(j.GetField("color"), {1.0f, 1.0f, 1.0f, 1.0f});
    }
    
    if (j.HasField("intensity")) {
        light.intensity = static_cast<float>(j.GetField("intensity").AsNumber());
    }
    
    if (j.HasField("range")) {
        light.range = static_cast<float>(j.GetField("range").AsNumber());
    }
    
    if (j.HasField("enabled")) {
        light.enabled = j.GetField("enabled").AsBool();
    }
    
    return light;
}

ECS::RotateComponent SceneLoader::ParseRotate(const JsonValue& j) {
    ECS::RotateComponent rotate;
    
    if (j.HasField("axis")) {
        rotate.axis = ParseVec3(j.GetField("axis"), {0.0f, 1.0f, 0.0f});
    }
    
    if (j.HasField("speed")) {
        rotate.speed = static_cast<float>(j.GetField("speed").AsNumber());
    }
    
    return rotate;
}

ECS::OrbitComponent SceneLoader::ParseOrbit(const JsonValue& j) {
    ECS::OrbitComponent orbit;
    
    if (j.HasField("center")) {
        orbit.center = ParseVec3(j.GetField("center"), {0.0f, 0.0f, 0.0f});
    }
    
    if (j.HasField("radius")) {
        orbit.radius = static_cast<float>(j.GetField("radius").AsNumber());
    }
    
    if (j.HasField("speed")) {
        orbit.speed = static_cast<float>(j.GetField("speed").AsNumber());
    }
    
    if (j.HasField("angle")) {
        orbit.angle = static_cast<float>(j.GetField("angle").AsNumber());
    }
    
    if (j.HasField("axis")) {
        orbit.axis = ParseVec3(j.GetField("axis"), {0.0f, 1.0f, 0.0f});
    }
    
    return orbit;
}

ECS::PlayerControllerComponent SceneLoader::ParsePlayerController(const JsonValue& j) {
    ECS::PlayerControllerComponent controller;
    
    if (j.HasField("moveSpeed")) {
        controller.moveSpeed = static_cast<float>(j.GetField("moveSpeed").AsNumber());
    }
    
    if (j.HasField("jumpForce")) {
        controller.jumpForce = static_cast<float>(j.GetField("jumpForce").AsNumber());
    }
    
    if (j.HasField("mouseSensitivity")) {
        controller.mouseSensitivity = static_cast<float>(j.GetField("mouseSensitivity").AsNumber());
    }
    
    if (j.HasField("cameraHeight")) {
        controller.cameraHeight = static_cast<float>(j.GetField("cameraHeight").AsNumber());
    }
    
    if (j.HasField("canJump")) {
        controller.canJump = j.GetField("canJump").AsBool();
    }
    
    return controller;
}

// ========================================
// Helper Methods
// ========================================

DirectX::XMFLOAT3 SceneLoader::ParseVec3(const JsonValue& arr, const DirectX::XMFLOAT3& defaultValue) {
    if (!arr.IsArray()) {
        return defaultValue;
    }
    
    if (arr.ArraySize() != 3) {
        throw std::runtime_error("Vec3 array must have exactly 3 elements");
    }
    
    return DirectX::XMFLOAT3{
        static_cast<float>(arr[0].AsNumber()),
        static_cast<float>(arr[1].AsNumber()),
        static_cast<float>(arr[2].AsNumber())
    };
}

DirectX::XMFLOAT4 SceneLoader::ParseVec4(const JsonValue& arr, const DirectX::XMFLOAT4& defaultValue) {
    if (!arr.IsArray()) {
        return defaultValue;
    }
    
    if (arr.ArraySize() != 4) {
        throw std::runtime_error("Vec4 array must have exactly 4 elements");
    }
    
    return DirectX::XMFLOAT4{
        static_cast<float>(arr[0].AsNumber()),
        static_cast<float>(arr[1].AsNumber()),
        static_cast<float>(arr[2].AsNumber()),
        static_cast<float>(arr[3].AsNumber())
    };
}
