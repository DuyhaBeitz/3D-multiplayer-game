#pragma once

#include <raylib.h>
#include <unordered_map>
#include <iostream>
#include <cstdint>

constexpr uint32_t R_MODEL_PLAYER = 0;

class Resources {
private:
    Resources() {
        m_default_model = LoadModel("assets/model.glb");
        m_models[R_MODEL_PLAYER] = LoadModel("assets/model.glb");
    }

    ~Resources() {
    }

    Model m_default_model;
    std::unordered_map<uint32_t, Model> m_models;

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

    Model& ModelFromKey(uint32_t key) {
        if (m_models.find(key) != m_models.end()) return m_models.at(key); 
        else {
            std::cerr << "No model found for key: " << key << std::endl;
            return m_default_model;
        }
    }
};