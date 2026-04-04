#pragma once

#include "Scene.hpp"
#include <memory>

#include "Scenes/Desert.hpp"
#include "Scenes/Green.hpp"
#include "Scenes/Forest.hpp"

class SceneManager {
private:
    std::unique_ptr<SceneBase> m_scene;
    Scenes m_scene_id;

public:
    SceneManager() {
        SetScene(default_scene);
    }

    SceneBase* GetScene() { return m_scene.get(); }
    const SceneBase* GetScene() const { return m_scene.get(); }

    void SetScene(Scenes scene_id) {
        switch (scene_id) {
        case Scenes::Desert:
            m_scene = std::make_unique<Desert>();
            break;

        case Scenes::Green:
            m_scene = std::make_unique<Green>();
            break;
        case Scenes::Forest:
        default:
            m_scene = std::make_unique<Forest>();
            break;
        }
        m_scene_id = scene_id;
    }

    void ChangeScene(Scenes scene_id) {
        if (scene_id == Scenes::None) return;
        m_scene->Unload();
        m_scene.reset();

        SetScene(scene_id);
        
        m_scene->Load();
    }

    Scenes GetSceneId() { return m_scene_id; }
};