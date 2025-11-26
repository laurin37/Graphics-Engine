#pragma once
#include <string>

namespace Config {
    namespace Paths {
        const std::wstring DefaultScene = L"../Assets/Scenes/default.json";
        const std::wstring DefaultFont = L"../Assets/Textures/bitmap/Minecraft.ttf";
        const std::string DefaultProjectileMesh = "../Assets/Models/basic/sphere.obj";
    }

    namespace UI {
        const std::wstring FontName = L"Minecraft";
        const float FontSize = 24.0f;
    }
}
