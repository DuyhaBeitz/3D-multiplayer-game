#pragma once

#include "Resources.hpp"

class Rendering {
private:
    Rendering() {
        Resources::Init();
    }

    ~Rendering() {
    }

public:
    Rendering(const Rendering&) = delete;
    Rendering& operator=(const Rendering&) = delete;

    static Rendering& Get() {
        static Rendering instance;
        return instance;
    }

    static void Init() {
        Rendering::Get();
    }


    void RenderModel(ModelKey model_key, Vector3 position, Vector3 rotationAxis = {0, 1, 0}, float rotationAngle = 0, Vector3 scale = {1, 1, 1}, Color tint = WHITE) {
        DrawModelEx(
            Resources::Get().ModelFromKey(model_key),
            position, rotationAxis, rotationAngle, scale, tint
        );
    }
};