#pragma once

#include "Resources.hpp"

class Rendering {
private:

    RenderTexture2D m_texture;

    Camera3D m_camera;

    Rendering();

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


    void RenderModel(ModelKey model_key, Vector3 position, Vector3 rotationAxis = {0, 1, 0}, float rotationAngle = 0, Vector3 scale = {1, 1, 1}, Color tint = WHITE);

    void SetCamera(const Camera3D& camera) { m_camera = camera; }
    const Camera3D& GetCamera() { return m_camera; }

    void BeginRendering() {BeginDrawing();}
    void EndRendering() {EndDrawing();}

    void BeginCameraMode() {BeginMode3D(m_camera);}
    void EndCameraMode() {EndMode3D();}


};