#pragma once

#include "Scene.hpp"
#include <memory>

#include "Scenes/Desert.hpp"

class SceneManager {
private:
    std::unique_ptr<SceneBase> m_scene;
public:

    SceneManager() {
        m_scene = std::make_unique<Desert>();
    }

    SceneBase* GetScene() { return m_scene.get(); }
    const SceneBase* GetScene() const { return m_scene.get(); }
};