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
    src/main.cpp
    src/AssetManager.cpp
    src/Bullet.cpp
    src/Camera.cpp
    src/Collider.cpp
    src/Crosshair.cpp
    src/FontLoader.cpp
    src/Game.cpp
    src/GameObject.cpp
    src/Graphics.cpp
    src/Gun.cpp
    src/HealthObject.cpp
    src/Input.cpp
    src/Logger.cpp
    src/Material.cpp
    src/Mesh.cpp
    src/ModelLoader.cpp
    src/PhysicsBody.cpp
    src/PhysicsSystem.cpp
    src/Player.cpp
    src/PostProcess.cpp
    src/Renderer.cpp
    src/Scene.cpp
    src/Shader.cpp
    src/SimpleFont.cpp
    src/Skybox.cpp
    src/TextureLoader.cpp
    src/Transform.cpp
    src/UIRenderer.cpp
    src/Window.cpp
)

# 5. Create the Executable (The .exe)
add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

# 6. Include Directories
# This lets you write #include "include/Graphics.h" or #include "Graphics.h"
target_include_directories(${PROJECT_NAME} PRIVATE src)

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
    "${CMAKE_SOURCE_DIR}/Assets"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets"
    COMMENT "Copying Assets to Output Directory..."
)