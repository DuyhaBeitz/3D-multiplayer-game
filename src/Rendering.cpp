#include "Rendering.hpp"

#define RT_WH 1024

Rendering::Rendering() {
    Resources::Init();

    m_texture = LoadRenderTexture(RT_WH, RT_WH);
    BeginTextureMode(m_texture);
    ClearBackground(BLANK);
    DrawText("Hello World!", 0, 0, 64, RED);
    EndTextureMode();
}

void Rendering::RenderModel(ModelKey model_key, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint) {
    DrawModelEx(
        Resources::Get().ModelFromKey(model_key),
        position, rotationAxis, rotationAngle, scale, tint
    );
}