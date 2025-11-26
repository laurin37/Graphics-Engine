#pragma once

#include "System.h"
#include <vector>
#include <memory>
#include <algorithm>

namespace ECS {

class SystemManager {
public:
    SystemManager() = default;

    template<typename T, typename... Args>
    T* AddSystem(ComponentManager& componentManager, Args&&... args) {
        auto system = std::make_unique<T>(componentManager, std::forward<Args>(args)...);
        T* systemPtr = system.get();
        m_systems.push_back(std::move(system));
        systemPtr->Init();
        return systemPtr;
    }

    void Update(float deltaTime) {
        for (auto& system : m_systems) {
            system->Update(deltaTime);
        }
    }

    // Optional: Get a system if needed
    template<typename T>
    T* GetSystem() {
        for (auto& system : m_systems) {
            if (T* casted = dynamic_cast<T*>(system.get())) {
                return casted;
            }
        }
        return nullptr;
    }

private:
    std::vector<std::unique_ptr<System>> m_systems;
};

} // namespace ECS
