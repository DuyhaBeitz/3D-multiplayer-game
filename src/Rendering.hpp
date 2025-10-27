#pragma once

#include "Resources.hpp"

class Rendering {
private:

    RenderTexture2D m_texture;

    Camera3D m_camera;

    Rendering() {
        Resources::Init();

        m_texture = LoadRenderTexture(100, 100);
        BeginTextureMode(m_texture);
        ClearBackground(BLANK);
        DrawText("Hello!", 0, 0, 64, RED);
        EndTextureMode();
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

    void RenderText3D(const Camera3D& camera) {
        DrawBillboard(camera, m_texture.texture, {0, 0, 0}, 1.f, WHITE);
    }

    void SetCamera(const Camera3D& camera) { m_camera = camera; }
    const Camera3D& GetCamera() { return m_camera; }

    void BeginRendering() {BeginDrawing();}
    void EndRendering() {EndDrawing();}

    void BeginCameraMode() {BeginMode3D(m_camera);}
    void EndCameraMode() {EndMode3D();}

    
};