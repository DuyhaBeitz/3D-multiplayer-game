#include "Rendering.hpp"

#define RT_WH 1024

Rendering::Rendering() {
    Resources::Init();

    m_texture = LoadRenderTexture(RT_WH, RT_WH);
    BeginTextureMode(m_texture);
    ClearBackground(BLANK);
    DrawText("Hello World!", 0, 0, 64, RED);
    EndTextureMode();


    R3D_Init(1000, 1000, 0);
    R3D_Light light = R3D_CreateLight(R3D_LIGHT_DIR);
    R3D_SetLightDirection(light, (Vector3){ -1, -1, -1 });
    R3D_SetLightActive(light, true);
}

void Rendering::RenderModel(ModelKey model_key, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale) {
    Resources::Get().ModelFromKey(model_key).Draw(position, rotationAxis, rotationAngle, scale);
}