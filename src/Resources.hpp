#pragma once

#include <raylib.h>
#include <raymath.h>
#include <unordered_map>
#include <iostream>
#include <cstdint>
#include <vector>

#include <r3d.h>
#include "Constants.hpp"

using ModelKey = uint16_t;
using FontKey = uint16_t;

std::vector<int> CodepointsFromStr(const char* chars);
Font LoadFontForCharacters(const char *fileName, int fontSize, const char* chars);

constexpr ModelKey R_MODEL_DEFAULT = 0;
constexpr ModelKey R_MODEL_PLAYER = 1;
constexpr ModelKey R_MODEL_CUBE_EXCLAMATION = 2;
constexpr ModelKey R_MODEL_HEIGHTMAP0 = 3;

constexpr FontKey R_FONT_DEFAULT = 0;

/*
Unlike other resources, Images can be loaded without window being initialized,
it's just pixel data.
Images can be used to initialize stuff like heightmap, dungeons etc. which has to be done on the server side too
So in order to load images, without initializing resources,
we store path to images, not images themselves
*/

#define P_HIEGHTMAP0_IMAGE_PATH "assets/Heightmap_01_Mountain.png"

struct AnimatedModelAlias {
    std::vector<R3D_Model> aliases;
    std::vector<R3D_ModelAnimation*> anims;
    float scale_multiplier = 1.f;
    Vector3 offset = {};

    int current_alias = 0;
    int anim_count = 0;

    void CreateSingle(R3D_Model model) {
        aliases.push_back(model);
    }

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
            position+offset, rotationAxis, rotationAngle, scale*scale_multiplier
        );
        current_alias++;
        current_alias = current_alias % aliases.size();
    }


    void SetMaterial(R3D_Material material) {
        for (int i = 0; i < aliases.size(); i++) {
            for (int j = 0; j < aliases[i].materialCount; j++) {
                aliases[i].materials[j] = material;
            }            
        }
    }

    //static R3D_Model CreateAlias(R3D_Model source);
};

inline R3D_Material CreateMaterial(Texture2D albedo, Texture2D normal) {
    R3D_Material mat = R3D_Material();
    mat.albedo.texture = albedo;
    mat.albedo.color = WHITE;

    mat.normal.texture = normal;
    mat.normal.scale = 1;
    
    mat.orm.metalness = 0;
    mat.orm.roughness = 0.7;
    mat.orm.occlusion = 0;

    mat.uvScale = {10, 10};

    return mat;
}

class Resources {
private:
    int max_font_size = 128; // any begger than that will be upscaled
    const char* supported_font_chars = 
    " `~!\"#$%&'()*+,-./0123456789:;<=>?@|_"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    // Latin-1 Supplement (accents for Western European languages)
    "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞß"
    "àáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ"
    // Spanish extra letters
    "¡¿"
    // French extra letters
    "Œœ"
    // German extra letters
    "ÄÖÜäöüß"
    // Portuguese extra letters
    "ÁÉÍÓÚÀÈÌÒÙÂÊÎÔÛÃÕÇáéíóúàèìòùâêîôûãõç"
    // Cyrillic (Russian, Ukrainian, etc.)
    "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"
    "абвгдеёжзийклмнопрстуфхцчшщъыьэюя"
    ;

    Resources() {
        AddModel(R_MODEL_DEFAULT, "assets/model.glb", 0);
        AddModel(R_MODEL_PLAYER, "assets/Solus_the_knight.gltf", 1);
        m_models[R_MODEL_PLAYER].scale_multiplier = 1.5;
        AddModel(R_MODEL_CUBE_EXCLAMATION, "assets/box_crate.glb", 0);
        //float s = 8.f;
        //m_models[R_MODEL_CUBE_EXCLAMATION].scale_multiplier = s;
        //m_models[R_MODEL_CUBE_EXCLAMATION].offset = Vector3{0, -1, 0}*s/2;

        AddHeightmapModel(R_MODEL_HEIGHTMAP0, P_HIEGHTMAP0_IMAGE_PATH, heightmap0_scale);

        m_models[R_MODEL_HEIGHTMAP0].SetMaterial(
            CreateMaterial(
                LoadTexture("assets/grass/GroundSand005_COL_1K.jpg"),
                LoadTexture("assets/grass/GroundSand005_NRM_1K.jpg")
            )
        );
        
        AddFont(R_FONT_DEFAULT,
            LoadFontForCharacters("assets/NotoSans-Black.ttf", max_font_size, supported_font_chars)
        );     
        SetTextureFilter(FontFromKey(R_FONT_DEFAULT).texture, TEXTURE_FILTER_ANISOTROPIC_16X);        
    }

    ~Resources() {
    }

    std::unordered_map<ModelKey, AnimatedModelAlias> m_models;
    std::unordered_map<FontKey, Font> m_fonts;

    void AddModel(ModelKey model_key, std::string filename, int num_aliases) {
        m_models[model_key].Load(filename, num_aliases);
    }

    void AddHeightmapModel(ModelKey model_key, std::string filename, Vector3 scale) {
        Image img = LoadImage(filename.c_str());
        R3D_Mesh mesh = R3D_GenMeshHeightmap(img, scale, true);
        m_models[model_key].CreateSingle(R3D_LoadModelFromMesh(&mesh));
        //m_models[model_key].aliases[0].materials->albedo.texture = LoadTextureFromImage(img);
        //m_models[model_key].aliases[0].materials->albedo.color = GREEN;
        UnloadImage(img);
    }

    void AddFont(FontKey font_key, Font font) {
        m_fonts[font_key] = font;
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

    Font FontFromKey(FontKey key) {
        if (m_fonts.find(key) != m_fonts.end()) return m_fonts.at(key); 
        else {
            std::cerr << "No font found for key: " << key << std::endl;
            return GetFontDefault();
        }
    }

    void ResetModelCurrentAlias() {
        for (auto& [model_key, aliased_model] : m_models) {
            aliased_model.current_alias = 0;
        }
    }
};