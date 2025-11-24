# 1. Minimum version of CMake required (3.20 is good for modern C++)
cmake_minimum_required(VERSION 3.20)

# 2. Project Name and Language
project(GeminiDXEngine LANGUAGES CXX)

# 3. Set C++ Standard to 23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 4. Explicitly list all source files
# This ensures CMake always includes all files (GLOB_RECURSE can miss new files)
set(SOURCE_FILES
    "GameEngineSolution/GameEngineProject/src/main.cpp"
    "GameEngineSolution/GameEngineProject/src/Application/Game.cpp"
    "GameEngineSolution/GameEngineProject/src/Application/Scene.cpp"
    "GameEngineSolution/GameEngineProject/src/EntityComponentSystem/Bullet.cpp"
    "GameEngineSolution/GameEngineProject/src/EntityComponentSystem/Camera.cpp"
    "GameEngineSolution/GameEngineProject/src/EntityComponentSystem/GameObject.cpp"
    "GameEngineSolution/GameEngineProject/src/EntityComponentSystem/Gun.cpp"
    "GameEngineSolution/GameEngineProject/src/EntityComponentSystem/HealthObject.cpp"
    "GameEngineSolution/GameEngineProject/src/EntityComponentSystem/Player.cpp"
    "GameEngineSolution/GameEngineProject/src/Input/Input.cpp"
    "GameEngineSolution/GameEngineProject/src/Physics/Collider.cpp"
    "GameEngineSolution/GameEngineProject/src/Physics/PhysicsBody.cpp"
    "GameEngineSolution/GameEngineProject/src/Physics/PhysicsSystem.cpp"
    "GameEngineSolution/GameEngineProject/src/Platform/Window.cpp"
    "GameEngineSolution/GameEngineProject/src/Renderer/BloomEffect.cpp"
    "GameEngineSolution/GameEngineProject/src/Renderer/Graphics.cpp"
    "GameEngineSolution/GameEngineProject/src/Renderer/Material.cpp"
    "GameEngineSolution/GameEngineProject/src/Renderer/Mesh.cpp"
    "GameEngineSolution/GameEngineProject/src/Renderer/PostProcess.cpp"
    "GameEngineSolution/GameEngineProject/src/Renderer/Renderer.cpp"
    "GameEngineSolution/GameEngineProject/src/Renderer/Skybox.cpp"
    "GameEngineSolution/GameEngineProject/src/ResourceManagement/AssetManager.cpp"
    "GameEngineSolution/GameEngineProject/src/ResourceManagement/FontLoader.cpp"
    "GameEngineSolution/GameEngineProject/src/ResourceManagement/ModelLoader.cpp"
    "GameEngineSolution/GameEngineProject/src/ResourceManagement/Shader.cpp"
    "GameEngineSolution/GameEngineProject/src/ResourceManagement/TextureLoader.cpp"
    "GameEngineSolution/GameEngineProject/src/UI/Crosshair.cpp"
    "GameEngineSolution/GameEngineProject/src/UI/SimpleFont.cpp"
    "GameEngineSolution/GameEngineProject/src/UI/UIRenderer.cpp"
    "GameEngineSolution/GameEngineProject/src/Utils/Logger.cpp"
    "GameEngineSolution/GameEngineProject/src/Utils/Transform.cpp"
    "GameEngineSolution/GameEngineProject/src/ECS/ComponentManager.cpp"
    "GameEngineSolution/GameEngineProject/src/ECS/Systems/ECSPhysicsSystem.cpp"
    "GameEngineSolution/GameEngineProject/src/ECS/Systems/ECSRenderSystem.cpp"
    "GameEngineSolution/GameEngineProject/src/ECS/Systems/ECSMovementSystem.cpp"
)

# 5. Create the Executable (The .exe)
add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

# 6. Include Directories
# This lets you write #include "include/Graphics.h" or #include "Graphics.h"
target_include_directories(${PROJECT_NAME} PRIVATE GameEngineSolution/GameEngineProject/include)

# 7. Link DirectX Libraries
# Windows usually finds these automatically.
target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11
    dxgi
    d3dcompiler
)

# 8. AUTOMATIC ASSET COPYING (The "Pro" Fix)
# This command runs after the build finishes. 
# It copies your 'Assets' folder right next to the new .exe.
# No more path errors!
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/GameEngineSolution/GameEngineProject/Assets"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets"
    COMMENT "Copying Assets to Output Directory..."
)