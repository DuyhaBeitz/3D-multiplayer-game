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

class InstancesData {
private:
    R3D_InstanceBuffer m_instances{};
    int m_instance_count = 0;

    Vector3* m_positions = nullptr;
    Quaternion* m_rotations = nullptr;
    Vector3* m_scales = nullptr;
    Color* m_colors = nullptr;

public:
    InstancesData() = default;
    InstancesData(int instance_count) : m_instance_count(instance_count) {
        m_instances = R3D_LoadInstanceBuffer(m_instance_count, R3D_INSTANCE_POSITION | R3D_INSTANCE_ROTATION | R3D_INSTANCE_SCALE | R3D_INSTANCE_COLOR);
        m_positions = reinterpret_cast<Vector3*>(R3D_MapInstances(m_instances, R3D_INSTANCE_POSITION));
        m_rotations = reinterpret_cast<Quaternion*>(R3D_MapInstances(m_instances, R3D_INSTANCE_ROTATION));
        m_scales = reinterpret_cast<Vector3*>(R3D_MapInstances(m_instances, R3D_INSTANCE_SCALE));
        m_colors = reinterpret_cast<Color*>(R3D_MapInstances(m_instances, R3D_INSTANCE_COLOR));

        for (int i = 0; i < instance_count; i++)
        {
            m_positions[i] = (Vector3) {0.0f, 0.0f, 0.0f};
            m_rotations[i] = QuaternionFromEuler(0.0f, 0.0f, 0.0f);
            m_scales[i] = (Vector3) {1.f, 1.f, 1.f};
            m_colors[i] = WHITE;
        }

        R3D_UnmapInstances(m_instances, R3D_INSTANCE_POSITION | R3D_INSTANCE_ROTATION | R3D_INSTANCE_SCALE | R3D_INSTANCE_COLOR);
    }
  
    void Unload() {
        R3D_UnloadInstanceBuffer(m_instances);
        m_instance_count = 0;

        m_positions = nullptr;
        m_rotations = nullptr;
        m_scales = nullptr;
        m_colors = nullptr;
    }

    R3D_InstanceBuffer GetBuffer() { return m_instances; }
    int GetCount() { return m_instance_count; }
    Vector3* GetPositions() { return m_positions; }
    Quaternion* GetRotations() { return m_rotations; }
    Vector3* GetScales() { return m_scales; }
    Color* GetColors() { return m_colors; }

    void SetPositions(std::vector<Vector3> positions) {
        Vector3* mapped = reinterpret_cast<Vector3*>(R3D_MapInstances(m_instances, R3D_INSTANCE_POSITION));
        if (positions.size() != m_instance_count) throw std::logic_error("SetPositions for instanced model received wrong number of positions");
        for (int i = 0; i < m_instance_count; i++) {
            mapped[i] = positions[i];
        }
        R3D_UnmapInstances(m_instances, R3D_INSTANCE_POSITION | R3D_INSTANCE_ROTATION | R3D_INSTANCE_SCALE | R3D_INSTANCE_COLOR);
    }

    void SetRotations(std::vector<Quaternion> rotations) {
        Quaternion* mapped = reinterpret_cast<Quaternion*>(R3D_MapInstances(m_instances, R3D_INSTANCE_ROTATION));
        if (rotations.size() != m_instance_count) throw std::logic_error("SetRotations for instanced model received wrong number of rotations");
        for (int i = 0; i < m_instance_count; i++) {
            mapped[i] = rotations[i];
        }
        R3D_UnmapInstances(m_instances, R3D_INSTANCE_POSITION | R3D_INSTANCE_ROTATION | R3D_INSTANCE_SCALE | R3D_INSTANCE_COLOR);
    }

    void SetScales(std::vector<Vector3> scales) {
        Vector3* mapped = reinterpret_cast<Vector3*>(R3D_MapInstances(m_instances, R3D_INSTANCE_SCALE));
        if (scales.size() != m_instance_count) throw std::logic_error("SetScales for instanced model received wrong number of scales");
        for (int i = 0; i < m_instance_count; i++) {
            mapped[i] = scales[i];
        }
        R3D_UnmapInstances(m_instances, R3D_INSTANCE_POSITION | R3D_INSTANCE_ROTATION | R3D_INSTANCE_SCALE | R3D_INSTANCE_COLOR);
    }

    void SetColors(std::vector<Color> colors) {
        Color* mapped = reinterpret_cast<Color*>(R3D_MapInstances(m_instances, R3D_INSTANCE_COLOR));
        if (colors.size() != m_instance_count) throw std::logic_error("SetColors for instanced model received wrong number of colors");
        for (int i = 0; i < m_instance_count; i++) {
            mapped[i] = colors[i];
        }
        R3D_UnmapInstances(m_instances, R3D_INSTANCE_POSITION | R3D_INSTANCE_ROTATION | R3D_INSTANCE_SCALE | R3D_INSTANCE_COLOR);
    }
};

class ModelAliased {
private:
    std::vector<R3D_AnimationPlayer> m_anim_players = {};
    R3D_AnimationLib m_anim_lib = {};
    R3D_Model m_model = {};
    
    float m_scale_multiplier = 1.f;
    Vector3 m_offset = {};

    int m_current_alias = 0;
    bool m_animated = false;
    bool m_instanced = false;

    InstancesData m_instances_data{};

public:
    ~ModelAliased() { Unload(); }
    ModelAliased() = default;

    void Unload() {
        std::cout << "Unloading model aliased" << std::endl;
        if (IsAnimated()) {
            for (auto& anim_player : m_anim_players) {
                R3D_UnloadAnimationPlayer(anim_player);
            }
            R3D_UnloadAnimationLib(m_anim_lib);
        }
        if (IsInstanced()) {
            m_instances_data.Unload();
        }
        R3D_UnloadModel(m_model, true);
    }

    void FromMeshNonAnimated(R3D_Mesh mesh) {
        m_model = R3D_LoadModelFromMesh(mesh);
    }

    void LoadInstanced(std::string filename, int instance_count) {
        m_instanced = true;
        m_instances_data = InstancesData(instance_count);

        std::cout << "Loading model instanced: " << filename << std::endl;
        if (instance_count < 0) {
            std::cerr << "Loading model with instances, but instance_count is < 0" << std::endl;
            throw std::logic_error("Loading < 0 instanes");
            return;
        }
        std::cout << "With " << instance_count << " instances" << std::endl;

        m_model = R3D_LoadModel(filename.c_str());
    }

    void LoadNonAnimated(std::string filename) {
        std::cout << "Loading model non-animated: " << filename << std::endl;
        m_model = R3D_LoadModel(filename.c_str());
        std::cout << "Successfully loaded model" << std::endl;
    }

    void LoadAnimated(std::string filename, int num_aliases) {
        m_animated  = true;

        std::cout << "Loading model animated: " << filename << std::endl;
        if (num_aliases < 0) {
            std::cerr << "Loading AnimatedModelAlias, but num_aliases is < 0" << std::endl;
            throw std::logic_error("Attemting to load non animated model as animated?");
            return;
        }
        std::cout << "With " << num_aliases << " aliases" << std::endl;
        m_anim_players.resize(num_aliases);

        const char* s = filename.c_str();
        {
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

    void DrawInstanced() {
        R3D_DrawModelInstanced(m_model, m_instances_data.GetBuffer(), m_instances_data.GetCount());
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

    bool IsInstanced() {
        return m_instanced;
    }

    void SetScale(float scale) {
        m_scale_multiplier = scale;
    }

    void SetCurrentAlias(int current_alias) {
        m_current_alias = current_alias;
    }

    InstancesData* GetInstancesData() { return &m_instances_data; }
};

inline R3D_Material CreateMaterial(Texture2D albedo, Texture2D normal, float normal_scale = 1.0f, float uv_scale = 1.0f) {
    R3D_Material mat = R3D_Material();
    mat.albedo.texture = albedo;
    mat.albedo.color = WHITE;

    mat.normal.texture = normal;
    mat.normal.scale = normal_scale;
    
    mat.orm.metalness = 0;
    mat.orm.roughness = 0.7;
    mat.orm.occlusion = 0;

    mat.uvScale = Vector2{uv_scale, uv_scale};

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
        SetModelNonAnimated(R_MODEL_DEFAULT, "assets/model.glb");
        SetModelAnimated(R_MODEL_PLAYER, "assets/Solus_the_knight.gltf", 10);
        m_models[R_MODEL_PLAYER].SetScale(1.5f);
        SetModelNonAnimated(R_MODEL_CUBE_EXCLAMATION, "assets/box_crate.glb");
        SetModelNonAnimated(R_MODEL_FOOTBALL, "assets/football_ball.glb");
        m_models[R_MODEL_FOOTBALL].SetScale(9.1f);

        SetInstancedModel(R_MODEL_TREE, "assets/palm_tree_realistic.glb", 250);
        SetInstancedModel(R_MODEL_GRASS, "assets/grass.glb", 500);

        SetHeightmapModel(R_MODEL_HEIGHTMAP0, P_HIEGHTMAP0_IMAGE_PATH, heightmap0_scale);

        m_models[R_MODEL_HEIGHTMAP0].SetMaterial(
            CreateMaterial(
                LoadTexture("assets/sand/GroundSand005_COL_1K.jpg"),
                LoadTexture("assets/sand/GroundSand005_NRM_1K.jpg"),
                1.0f,
                10.0f
            )
        );
        
        SetFont(R_FONT_DEFAULT,
            LoadFontForCharacters("assets/NotoSans-Black.ttf", max_font_size, supported_font_chars)
        );
        SetTextureFilter(FontFromKey(R_FONT_DEFAULT).texture, TEXTURE_FILTER_ANISOTROPIC_16X);        

        std::cout << "Successfully loaded resources" << std::endl;
    }

    ~Resources() {
    }

    std::unordered_map<ModelKey, ModelAliased> m_models;
    std::unordered_map<FontKey, Font> m_fonts;

    void SetInstancedModel(ModelKey model_key, std::string filename, int num_instances) {
        if (m_models.find(model_key) != m_models.end()) m_models.erase(model_key);
        m_models[model_key].LoadInstanced(filename, num_instances);
    }

    void SetModelNonAnimated(ModelKey model_key, std::string filename) {
        if (m_models.find(model_key) != m_models.end()) m_models.erase(model_key);
        m_models[model_key].LoadNonAnimated(filename);
    }

    void SetModelAnimated(ModelKey model_key, std::string filename, int num_aliases) {
        if (m_models.find(model_key) != m_models.end()) m_models.erase(model_key);
        m_models[model_key].LoadAnimated(filename, num_aliases);
    }

    void SetHeightmapModel(ModelKey model_key, std::string filename, Vector3 scale) {
        if (m_models.find(model_key) != m_models.end()) m_models.erase(model_key);

        Image img = LoadImage(filename.c_str());
        R3D_Mesh mesh = R3D_GenMeshHeightmap(img, scale);

        m_models[model_key].FromMeshNonAnimated(mesh);
        UnloadImage(img);
    }

    void SetFont(FontKey font_key, Font font) {
        if (m_fonts.find(font_key) != m_fonts.end()) {
            UnloadFont(m_fonts[font_key]);
            m_fonts.erase(font_key);
        }
        m_fonts[font_key] = font;
    }

public:
    void Unload() {
        m_models.clear();

        for (auto&& [key, font] : m_fonts) {
            UnloadFont(font);
        }
        m_fonts.clear();
    }

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
            throw std::range_error(TextFormat("model key is not found: %d", key));
        }
    }

    Font FontFromKey(FontKey key) {
        if (m_fonts.find(key) != m_fonts.end()) return m_fonts.at(key); 
        else {
            throw std::range_error(TextFormat("font key is not found: %d", key));
        }
    }

    void ResetModelCurrentAlias() {
        for (auto& [model_key, aliased_model] : m_models) {
            aliased_model.SetCurrentAlias(0);
        }
    }
};