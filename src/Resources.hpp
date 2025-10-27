#pragma once

#include <raylib.h>
#include <unordered_map>
#include <iostream>
#include <cstdint>

using ModelKey = uint16_t;

constexpr ModelKey R_MODEL_DEFAULT = 0;
constexpr ModelKey R_MODEL_PLAYER = 1;

struct ModelAnimationData {
    int anim_count;
    ModelAnimation* model_animations;
};

class Resources {
private:
    Resources() {
        AddModel(R_MODEL_DEFAULT, "assets/model.glb");

        AddModelWithAnimation(R_MODEL_PLAYER, "assets/Character.gltf");
    }

    ~Resources() {
    }

    std::unordered_map<ModelKey, Model> m_models;
    std::unordered_map<ModelKey, ModelAnimationData> m_model_animations;

    void AddModel(ModelKey model_key, std::string filename) {
        m_models[model_key] = LoadModel(filename.c_str());
        m_model_animations[model_key] = ModelAnimationData{
            0, nullptr
        };
    }

    void AddModelWithAnimation(ModelKey model_key, std::string filename) {
        std::cout << "Adding model with animations" << std::endl;
        m_models[model_key] = LoadModel(filename.c_str());

        m_model_animations[model_key] = ModelAnimationData{};
        m_model_animations[model_key].model_animations = LoadModelAnimations(filename.c_str(), &m_model_animations[model_key].anim_count);

        Model& model = m_models[model_key];
        std::cout << "Model bones: " << model.boneCount << std::endl;
        std::cout << "Model meshes: " << model.meshCount << std::endl;
        std::cout << "Anim count: " << m_model_animations[model_key].anim_count << std::endl;
        for (int i = 0; i < m_model_animations[model_key].anim_count; i++) {
            std::cout << "Anim " << i << " bones: " << m_model_animations[model_key].model_animations[i].boneCount
                    << " frames: " << m_model_animations[model_key].model_animations[i].frameCount << std::endl;
        }

        for (int i = 0; i < model.meshCount; i++) {
            Mesh m = model.meshes[i];
            std::cout << "Mesh " << i << " bones: "
                    << (m.boneIds ? "OK" : "NULL")
                    << ", weights: "
                    << (m.boneWeights ? "OK" : "NULL")
                    << std::endl;
        }
        std::cout << "Done adding model with animations" << std::endl;
    }

public:
    Resources(const Resources&) = delete;
    Resources& operator=(const Resources&) = delete;

    static Resources& Get() {
        static Resources instance;
        return instance;
    }

    static void Init() {
        Resources::Get();
    }

    Model& ModelFromKey(ModelKey key) {
        if (m_models.find(key) != m_models.end()) return m_models.at(key); 
        else {
            std::cerr << "No model found for key: " << key << std::endl;
            return m_models[R_MODEL_DEFAULT];
        }
    }

    ModelAnimationData& ModelAnimationDataFromKey(ModelKey key) {
        if (m_model_animations.find(key) != m_model_animations.end()) return m_model_animations.at(key); 
        else {
            std::cerr << "No model animation found for key: " << key << std::endl;
            return m_model_animations[R_MODEL_DEFAULT];
        }
    }
};