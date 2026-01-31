#pragma once

#include <raylib.h>
#include <raymath.h>
#include <unordered_map>
#include <iostream>
#include <vector>

#include <r3d/r3d.h>
#include "Constants.hpp"

#include "ResourceData.hpp"

std::vector<int> CodepointsFromStr(const char* chars);
Font LoadFontForCharacters(const char *fileName, int fontSize, const char* chars);

/*
Unlike other resources, Images can be loaded without window being initialized,
it's just pixel data.
Images can be used to initialize stuff like heightmap, dungeons etc. which has to be done on the server side too
So in order to load images, without initializing resources,
we store path to images, not images themselves
*/


class ModelAliased {
private:
    std::vector<R3D_AnimationPlayer> m_anim_players = {};
    R3D_AnimationLib m_anim_lib = {};
    R3D_Model m_model = {};
    
    float m_scale_multiplier = 1.f;
    Vector3 m_offset = {};

    int m_current_alias = 0;
    bool m_animated = false;

public:
    ModelAliased() = default;

    void FromMeshNonAnimated(R3D_Mesh mesh) {
        m_animated = false;
        m_model = R3D_LoadModelFromMesh(mesh);
    }

    void LoadNonAnimated(std::string filename) {
        std::cout << "Loading model non-animated: " << filename << std::endl;
        m_model = R3D_LoadModel(filename.c_str());
        m_animated = false;
    }

    void LoadAnimated(std::string filename, int num_aliases) {
        m_animated  = true;

        std::cout << "Loading model animated: " << filename << std::endl;
        std::cout << "With " << num_aliases << " aliases" << std::endl;
        if (num_aliases < 0) {
            std::cerr << "Loading AnimatedModelAlias, but num_aliases is < 0" << std::endl;
            throw std::logic_error("Attemting to load non animated model as animated?");
            return;
        }
        m_anim_players.resize(num_aliases);

        const char* s = filename.c_str();
        {
        // Load model, anims into temp pointer, write temp pointer into model
        m_model = R3D_LoadModel(s);
        m_anim_lib = R3D_LoadAnimationLib(s);
        if (m_anim_lib.count == 0 || !m_anim_lib.animations) {
            std::cerr << "Failed to load anim lib, or it's empty" << std::endl;
            throw std::logic_error("Attemting to load non animated model as animated?");
            return;
        }
        }

        for (int i = 0; i < num_aliases; i++) {
            m_anim_players[i] = R3D_LoadAnimationPlayer(m_model.skeleton, m_anim_lib);

            for (int j = 0; j < m_anim_lib.count; j++) {
                R3D_SetAnimationLoop(&m_anim_players[i], j, true);
            }
        }

        PrintDebug();
        
        if (m_anim_players.size() != num_aliases) {
            throw std::runtime_error("m_anim_players.size() != num_aliases");
            return;
        }

        std::cout << "Successfully loaded model " << filename << '\n' << std::endl;
    }

    void Update(float dt) {
        R3D_UpdateAnimationPlayer(&m_anim_players[m_current_alias], dt);        
    }

    void SetAnim(int anim_id) {
        for (int j = 0; j < m_anim_lib.count; j++) { // can be improved by storing previous anim
            R3D_SetAnimationWeight(&m_anim_players[m_current_alias],  j, 0.f);
        }        
        R3D_SetAnimationWeight(&m_anim_players[m_current_alias], anim_id, 1.f);

        R3D_PlayAnimation(&m_anim_players[m_current_alias], anim_id);
    }

    void Draw(Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale) {
        Quaternion q = QuaternionFromAxisAngle(rotationAxis, rotationAngle);
        if (IsAnimated()) {
            if (m_anim_players.size() > 0) {
                R3D_DrawAnimatedModelEx(m_model, m_anim_players[m_current_alias], position+m_offset, q, scale*m_scale_multiplier);    
                m_current_alias++;
                m_current_alias = m_current_alias % m_anim_players.size();
            }
            else {
                std::cout << "m_anim_players.size() <= 0" << std::endl;
            }
        }
        else {
            R3D_DrawModelEx(
                m_model,
                position+m_offset, q, scale*m_scale_multiplier
            );
        }
    }

    void SetMaterial(R3D_Material material) {
        for (int i = 0; i < m_model.materialCount; i++) {
            m_model.materials[i] = material;
        }        
    }

    void PrintDebug() {
        std::cout << "\tBone count: " << m_model.skeleton.boneCount << std::endl;
        std::cout << "\tMesh count: " << m_model.meshCount << std::endl;
        std::cout << "\tAnim count: " << m_anim_lib.count << std::endl;
        for (int i = 0; i < m_anim_lib.count; i++) {
            std::cout
            << "\tAnim: " << i << " " << m_anim_lib.animations[i].name 
            << "\tduration: " << m_anim_lib.animations[i].duration << std::endl;
        }
    }

    bool IsAnimated() {
        return m_animated;
    }

    void SetScale(float scale) {
        m_scale_multiplier = scale;
    }

    void SetCurrentAlias(int current_alias) {
        m_current_alias = current_alias;
    }
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
    " `~!\"#$%&'()*+,-./0123456789:;<=>?@|_/\\^"
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
        AddModelNonAnimated(R_MODEL_DEFAULT, "assets/model.glb");
        AddModelAnimated(R_MODEL_PLAYER, "assets/Solus_the_knight.gltf", 10);
        m_models[R_MODEL_PLAYER].SetScale(1.5f);
        AddModelNonAnimated(R_MODEL_CUBE_EXCLAMATION, "assets/box_crate.glb");
        AddModelNonAnimated(R_MODEL_FOOTBALL, "assets/football_ball.glb");
        float s = 9.1f;
        m_models[R_MODEL_FOOTBALL].SetScale(s);
        //m_models[R_MODEL_CUBE_EXCLAMATION].m_offset = Vector3{0, -1, 0}*s/2;

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

        std::cout << "Successfully loaded resources" << std::endl;
    }

    ~Resources() {
    }

    std::unordered_map<ModelKey, ModelAliased> m_models;
    std::unordered_map<FontKey, Font> m_fonts;

    void AddModelNonAnimated(ModelKey model_key, std::string filename) {
        m_models[model_key].LoadNonAnimated(filename);
    }

    void AddModelAnimated(ModelKey model_key, std::string filename, int num_aliases) {
        m_models[model_key].LoadAnimated(filename, num_aliases);
    }

    void AddHeightmapModel(ModelKey model_key, std::string filename, Vector3 scale) {
        Image img = LoadImage(filename.c_str());
        R3D_Mesh mesh = R3D_GenMeshHeightmap(img, scale);

        m_models[model_key].FromMeshNonAnimated(mesh);
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

    ModelAliased& ModelFromKey(ModelKey key) {
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
            aliased_model.SetCurrentAlias(0);
        }
    }
};