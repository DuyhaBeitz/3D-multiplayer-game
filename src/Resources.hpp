#pragma once

#include <raylib.h>
#include <unordered_map>
#include <iostream>
#include <cstdint>
#include <vector>

#include <r3d.h>
#include "Constants.hpp"

using ModelKey = uint16_t;

constexpr ModelKey R_MODEL_DEFAULT = 0;
constexpr ModelKey R_MODEL_PLAYER = 1;
constexpr ModelKey R_MODEL_CUBE_EXCLAMATION = 2;

struct AnimatedModelAlias {
    std::vector<R3D_Model> aliases;
    std::vector<R3D_ModelAnimation*> anims;
    int current_alias = 0;
    int anim_count = 0;

    void Load(std::string filename, int num_aliases) {
        aliases.clear();
        const char* s = filename.c_str();
        {
            anims.push_back(R3D_LoadModelAnimations(s, &anim_count, iters_per_sec));
            aliases.push_back(R3D_LoadModel(s));
            aliases[0].anim = &anims[0][0];

            R3D_Model& model = aliases[0];
            std::cout << "Model bones: " << model.boneCount << std::endl;
            std::cout << "Model meshes: " << model.meshCount << std::endl;
            std::cout << "Anim count: " << anim_count << std::endl;
            for (int i = 0; i < anim_count; i++) {
                std::cout << "Anim " << i << " bones: " << model.boneCount
                        << " frames: " << model.anim[i].frameCount << std::endl;
            }
        }

        for (int i = 1; i < num_aliases; i++) {
            anims.push_back(R3D_LoadModelAnimations(s, &anim_count, iters_per_sec));
            aliases.push_back(R3D_LoadModel(s));
            aliases[i].anim = &anims[i][0];
        }
    }

    void IncAnimFrame(int delta_frame = 1) {
        aliases[current_alias].animFrame += delta_frame;
    }

    void SetAnimFrame(int frame) {
        aliases[current_alias].animFrame = frame;
    }

    void SetAnim(int anim_id) {
        aliases[current_alias].anim = &anims[current_alias][anim_id];
    }

    void Draw(Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale) {
        R3D_DrawModelEx(
            &aliases[current_alias],
            position, rotationAxis, rotationAngle, scale
        );
        current_alias++;
        current_alias = current_alias % aliases.size();
    }

    //static R3D_Model CreateAlias(R3D_Model source);
};

class Resources {
private:
    Resources() {
        AddModel(R_MODEL_DEFAULT, "assets/model.glb", 0);
        AddModel(R_MODEL_PLAYER, "assets/Character.gltf", 10);
        AddModel(R_MODEL_CUBE_EXCLAMATION, "assets/Cube_Exclamation.gltf", 0);
    }

    ~Resources() {
    }

    std::unordered_map<ModelKey, AnimatedModelAlias> m_models;

    void AddModel(ModelKey model_key, std::string filename, int num_aliases) {
        m_models[model_key].Load(filename, 10);
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

    AnimatedModelAlias& ModelFromKey(ModelKey key) {
        if (m_models.find(key) != m_models.end()) return m_models.at(key); 
        else {
            std::cerr << "No model found for key: " << key << std::endl;
            return m_models[R_MODEL_DEFAULT];
        }
    }

    void ResetModelCurrentAlias() {
        for (auto& [model_key, aliased_model] : m_models) {
            aliased_model.current_alias = 0;
        }
    }
};