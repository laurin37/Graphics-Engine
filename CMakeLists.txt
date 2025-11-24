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
    "GameEngineSolution/GameEngineProject/src/AssetManager.cpp"
    "GameEngineSolution/GameEngineProject/src/BloomEffect.cpp"
    "GameEngineSolution/GameEngineProject/src/Bullet.cpp"
    "GameEngineSolution/GameEngineProject/src/Camera.cpp"
    "GameEngineSolution/GameEngineProject/src/Collider.cpp"
    "GameEngineSolution/GameEngineProject/src/Crosshair.cpp"
    "GameEngineSolution/GameEngineProject/src/FontLoader.cpp"
    "GameEngineSolution/GameEngineProject/src/Game.cpp"
    "GameEngineSolution/GameEngineProject/src/GameObject.cpp"
    "GameEngineSolution/GameEngineProject/src/Graphics.cpp"
    "GameEngineSolution/GameEngineProject/src/Gun.cpp"
    "GameEngineSolution/GameEngineProject/src/HealthObject.cpp"
    "GameEngineSolution/GameEngineProject/src/Input.cpp"
    "GameEngineSolution/GameEngineProject/src/Logger.cpp"
    "GameEngineSolution/GameEngineProject/src/Material.cpp"
    "GameEngineSolution/GameEngineProject/src/Mesh.cpp"
    "GameEngineSolution/GameEngineProject/src/ModelLoader.cpp"
    "GameEngineSolution/GameEngineProject/src/PhysicsBody.cpp"
    "GameEngineSolution/GameEngineProject/src/PhysicsSystem.cpp"
    "GameEngineSolution/GameEngineProject/src/Player.cpp"
    "GameEngineSolution/GameEngineProject/src/PostProcess.cpp"
    "GameEngineSolution/GameEngineProject/src/Renderer.cpp"
    "GameEngineSolution/GameEngineProject/src/Scene.cpp"
    "GameEngineSolution/GameEngineProject/src/Shader.cpp"
    "GameEngineSolution/GameEngineProject/src/SimpleFont.cpp"
    "GameEngineSolution/GameEngineProject/src/Skybox.cpp"
    "GameEngineSolution/GameEngineProject/src/TextureLoader.cpp"
    "GameEngineSolution/GameEngineProject/src/Transform.cpp"
    "GameEngineSolution/GameEngineProject/src/UIRenderer.cpp"
    "GameEngineSolution/GameEngineProject/src/Window.cpp"
)

# 5. Create the Executable (The .exe)
add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

# 6. Include Directories
# This lets you write #include "include/Graphics.h" or #include "Graphics.h"
target_include_directories(${PROJECT_NAME} PRIVATE GameEngineSolution/GameEngineProject/src)

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