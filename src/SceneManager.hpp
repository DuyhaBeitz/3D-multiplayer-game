#pragma once

#include "Scene.hpp"
#include <memory>

#include "Scenes/Desert.hpp"
#include "Scenes/Green.hpp"

class SceneManager {
private:
    std::unique_ptr<SceneBase> m_scene;
public:

    SceneManager() {
        m_scene = std::make_unique<Desert>();
        //m_scene = std::make_unique<Green>();
    }

    SceneBase* GetScene() { return m_scene.get(); }
    const SceneBase* GetScene() const { return m_scene.get(); }
    void ChangeScene(Scenes scene_id) {
        if (scene_id == Scenes::None) return;
        m_scene->Unload();
        m_scene.reset();

        switch (scene_id) {
        case Scenes::Desert:
            m_scene = std::make_unique<Desert>();
            break;

        case Scenes::Green:
        default:
            m_scene = std::make_unique<Green>();
            break;
        }
        
        m_scene->Load();
    }
};