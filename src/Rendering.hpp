#pragma once

#include "Resources.hpp"
#include <rlgl.h>
#include <raymath.h>

void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint);
// Draw a 2D text in 3D space
void DrawText3D(Font font, const char *text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint);

inline void DrawText3DEx(Font font, const char *text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint, float yaw, bool centered) {
    rlPushMatrix();
        float ryaw = yaw*PI/180;
        float textLength = MeasureTextEx(font, text, fontSize, fontSpacing).x;

        if (centered) {
            rlTranslatef(position.x-cos(-ryaw)*textLength/2, position.y+fontSize/2, position.z-sin(-ryaw)*textLength/2);
        }
        else {
            rlTranslatef(position.x, position.y, position.z);
        }
            
        rlRotatef(yaw, 0.0f, 1.0f, 0.0f);
               
        
        rlRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        DrawText3D(font, text, (Vector3){0, 0, 0}, fontSize, fontSpacing, lineSpacing, backface, tint);
    rlPopMatrix();
}

struct TextDrawingData {
    const char* text;
    Vector3 center;
    float yaw;
    float font_size;

    void Draw() {
        DrawText3DEx(Resources::Get().FontFromKey(R_FONT_DEFAULT), text, center, font_size, 1, 1, true, GOLD, yaw, true);
        //DrawTextEx(Resources::Get().FontFromKey(R_FONT_DEFAULT), text, Vector2{100, 100}, font_size, 1, GOLD);
    }
};

class Rendering {
private:

    RenderTexture2D m_texture;

    Camera3D m_camera;

    Rendering();

    ~Rendering() {
    }

    // primitives to render
    std::vector<std::pair<Vector3, Vector3>> m_cubes_to_render; // center, half_size
    std::vector<std::pair<Vector3, float>> m_spheres_to_render; // center, rad
    std::vector<TextDrawingData> m_texts_to_draw;

    const std::vector<Color> m_colors = {
        RED, GREEN, BLUE, YELLOW, PURPLE, VIOLET, BROWN, MAGENTA, GOLD, PINK
    };
    int m_current_color = 0;

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


    void RenderModel(ModelKey model_key, Vector3 position, Vector3 rotationAxis = {0, 1, 0}, float rotationAngle = 0, Vector3 scale = {1, 1, 1});

    void SetCamera(const Camera3D& camera) { m_camera = camera; }
    const Camera3D& GetCamera() { return m_camera; }

    void RenderPrimitiveCube(Vector3 center, Vector3 half_size) {
        m_cubes_to_render.push_back({center, half_size});
    }

    void RenderPrimitiveSphere(Vector3 center, float radius) {
        m_spheres_to_render.push_back({center, radius});
    }

    void RenderText(const char* text, Vector3 center, float yaw, float font_size) {
        m_texts_to_draw.push_back({text, center, yaw, font_size});
    }

    void BeginRendering() {
        R3D_Begin(m_camera);
    }
    
    void EndRendering() {
        R3D_End();
        Resources::Get().ResetModelCurrentAlias();
    }

    /*************************************************/

    void DrawPrimitives() {
        DrawGrid(100, 10);
        m_current_color = 0;
        for (auto& [center, half_size] : m_cubes_to_render) {
            DrawCube(center, half_size.x*2, half_size.y*2, half_size.z*2, m_colors[m_current_color]);
            m_current_color = (m_current_color+1) % m_colors.size();
        }
        for (auto& [center, radius] : m_spheres_to_render) {
            DrawSphere(center, radius, m_colors[m_current_color]);
            m_current_color = (m_current_color+1) % m_colors.size();
        }
        m_cubes_to_render.clear();
        m_spheres_to_render.clear();
    }

    void DrawTexts() {
        for (auto& text_data : m_texts_to_draw) {
            text_data.Draw();
        }
        m_texts_to_draw.clear();
    }

    void EnableCameraBasic() {
        BeginMode3D(m_camera);
    }

    void DisableCameraBasic() {
        EndMode3D();
    }
};