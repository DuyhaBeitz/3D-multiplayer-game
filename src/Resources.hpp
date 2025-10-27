#pragma once

#include <raylib.h>
#include <unordered_map>
#include <iostream>
#include <cstdint>

using ModelKey = uint16_t;

constexpr ModelKey R_MODEL_DEFAULT = 0;
constexpr ModelKey R_MODEL_PLAYER = 1;

class Resources {
private:
    Resources() {
        m_models[R_MODEL_DEFAULT] = LoadModel("assets/model.glb");
        m_models[R_MODEL_PLAYER] = LoadModel("assets/model.glb");
    }

    ~Resources() {
    }

    std::unordered_map<ModelKey, Model> m_models;

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
};