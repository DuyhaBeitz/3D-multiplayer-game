#include "Rendering.hpp"

#define RT_WH 1024

#define LETTER_BOUNDRY_SIZE     0.25f
#define TEXT_MAX_LAYERS         32
#define LETTER_BOUNDRY_COLOR    VIOLET
bool SHOW_LETTER_BOUNDRY = false;
bool SHOW_TEXT_BOUNDRY = false;

Rendering::Rendering() {
    R3D_Init(GetScreenWidth(), GetScreenHeight());
    Resources::Init();
    
    R3D_ShadowCastMode(R3D_SHADOW_CAST_FRONT_FACES);

    R3D_Cubemap cubemap = R3D_LoadCubemap("assets/skybox_1.png", R3D_CUBEMAP_LAYOUT_AUTO_DETECT);
    R3D_ENVIRONMENT_SET(background.skyBlur, 0.0f);

    R3D_ENVIRONMENT_SET(background.energy, 0.7f);
    R3D_ENVIRONMENT_SET(background.sky, cubemap);

    // Setup environment ambient
    R3D_AmbientMap ambientMap = R3D_GenAmbientMap(cubemap, R3D_AMBIENT_ILLUMINATION);
    R3D_ENVIRONMENT_SET(ambient.map, ambientMap);
    R3D_ENVIRONMENT_SET(ambient.energy, 0.3f);

    // Setup tonemapping
    R3D_ENVIRONMENT_SET(tonemap.mode, R3D_TONEMAP_FILMIC);
    R3D_ENVIRONMENT_SET(tonemap.exposure, 1.0f);

    //Create directional light with shadows
    R3D_Light light = R3D_CreateLight(R3D_LIGHT_DIR);
    R3D_SetLightDirection(light, (Vector3){ -1, -0.5, -1 });
    R3D_SetShadowUpdateMode(light, R3D_SHADOW_UPDATE_INTERVAL);
    R3D_SetLightActive(light, true);
    R3D_SetLightRange(light, 500.0f);
    R3D_SetShadowSoftness(light, 0.0f);
    R3D_SetShadowDepthBias(light, 0.01f);
    R3D_EnableShadow(light);    
    
    // FOG
    R3D_ENVIRONMENT_SET(fog.mode, R3D_FOG_EXP2);
    R3D_ENVIRONMENT_SET(fog.color, GRAY);
    R3D_ENVIRONMENT_SET(fog.start, 3.0f);
    R3D_ENVIRONMENT_SET(fog.end, 50.0f);
    R3D_ENVIRONMENT_SET(fog.density, 0.001f);
    R3D_ENVIRONMENT_SET(fog.skyAffect, 0.5f);

    // BLOOM
    // R3D_ENVIRONMENT_SET(bloom.mode, R3D_BLOOM_ADDITIVE);
    // R3D_ENVIRONMENT_SET(bloom.levels, 0.5f);
    // R3D_ENVIRONMENT_SET(bloom.intensity, 0.05f);
    // R3D_ENVIRONMENT_SET(bloom.threshold, 0.0f);
    // R3D_ENVIRONMENT_SET(bloom.softThreshold, 0.5f);
    // R3D_ENVIRONMENT_SET(bloom.filterRadius, 1.0f);

    { // For some reason need with the newer version of r3d
    BeginDrawing();
    Camera3D cam {};
    R3D_Begin(cam);
    R3D_End();
    EndDrawing();
    } 
}

void Rendering::RenderModel(ModelKey model_key, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale) {
    Resources::Get().ModelFromKey(model_key).Draw(position, rotationAxis, rotationAngle, scale);
}

void Rendering::RenderInstancedModel(ModelKey model_key) {
    Resources::Get().ModelFromKey(model_key).DrawInstanced();
}

// Draw codepoint at specified position in 3D space
void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint)
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scale = fontSize/(float)font.baseSize;

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    position.x += (float)(font.glyphs[index].offsetX - font.glyphPadding)*scale;
    position.z += (float)(font.glyphs[index].offsetY - font.glyphPadding)*scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding, font.recs[index].y - (float)font.glyphPadding,
                         font.recs[index].width + 2.0f*font.glyphPadding, font.recs[index].height + 2.0f*font.glyphPadding };

    float width = (float)(font.recs[index].width + 2.0f*font.glyphPadding)*scale;
    float height = (float)(font.recs[index].height + 2.0f*font.glyphPadding)*scale;

    if (font.texture.id > 0)
    {
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 0.0f;

        // normalized texture coordinates of the glyph inside the font texture (0.0f -> 1.0f)
        const float tx = srcRec.x/font.texture.width;
        const float ty = srcRec.y/font.texture.height;
        const float tw = (srcRec.x+srcRec.width)/font.texture.width;
        const float th = (srcRec.y+srcRec.height)/font.texture.height;

        if (SHOW_LETTER_BOUNDRY) DrawCubeWiresV((Vector3){ position.x + width/2, position.y, position.z + height/2}, (Vector3){ width, LETTER_BOUNDRY_SIZE, height }, LETTER_BOUNDRY_COLOR);

        rlCheckRenderBatchLimit(4 + 4*backface);
        rlSetTexture(font.texture.id);

        rlPushMatrix();
            rlTranslatef(position.x, position.y, position.z);

            rlBegin(RL_QUADS);
                rlColor4ub(tint.r, tint.g, tint.b, tint.a);

                // Front Face
                rlNormal3f(0.0f, 1.0f, 0.0f);                                   // Normal Pointing Up
                rlTexCoord2f(tx, ty); rlVertex3f(x,         y, z);              // Top Left Of The Texture and Quad
                rlTexCoord2f(tx, th); rlVertex3f(x,         y, z + height);     // Bottom Left Of The Texture and Quad
                rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height);     // Bottom Right Of The Texture and Quad
                rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);              // Top Right Of The Texture and Quad

                if (backface)
                {
                    // Back Face
                    rlNormal3f(0.0f, -1.0f, 0.0f);                              // Normal Pointing Down
                    rlTexCoord2f(tx, ty); rlVertex3f(x,         y, z);          // Top Right Of The Texture and Quad
                    rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);          // Top Left Of The Texture and Quad
                    rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height); // Bottom Left Of The Texture and Quad
                    rlTexCoord2f(tx, th); rlVertex3f(x,         y, z + height); // Bottom Right Of The Texture and Quad
                }
            rlEnd();
        rlPopMatrix();

        rlSetTexture(0);
    }
}

// Draw a 2D text in 3D space
void DrawText3D(Font font, const char *text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint)
{
    int length = TextLength(text);          // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0.0f;               // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;               // Offset X to next character to draw

    float scale = fontSize/(float)font.baseSize;

    for (int i = 0; i < length;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            textOffsetY += fontSize + lineSpacing;
            textOffsetX = 0.0f;
        }
        else
        {
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                DrawTextCodepoint3D(font, codepoint, (Vector3){ position.x + textOffsetX, position.y, position.z + textOffsetY }, fontSize, backface, tint);
            }

            if (font.glyphs[index].advanceX == 0) textOffsetX += (float)font.recs[index].width*scale + fontSpacing;
            else textOffsetX += (float)font.glyphs[index].advanceX*scale + fontSpacing;
        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }
}
