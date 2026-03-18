#include "Resources.hpp"
#include <cstring>

std::vector<int> CodepointsFromStr(const char *chars) {
    std::vector<int> codepoints;
    int count = 0;

    int i = 0;
    while (chars[i]) {
        int bytes = 0;
        int cp = GetCodepoint(chars + i, &bytes);
        i += bytes;
        codepoints.push_back(cp);
    }    
    return codepoints;
}

Font LoadFontForCharacters(const char *fileName, int fontSize, const char *chars)
{
    std::vector<int> codepoints = CodepointsFromStr(chars);
    return LoadFontEx(fileName, fontSize, codepoints.data(), codepoints.size());
}

R3D_Model LoadR3DModelFromMesh(R3D_Mesh mesh) {
    R3D_Model model = {0};

    model.meshes = reinterpret_cast<R3D_Mesh*>(RL_MALLOC(sizeof(R3D_Mesh)));
    model.meshes[0] = mesh;
    model.meshCount = 1;

    model.materials = reinterpret_cast<R3D_Material*>(RL_MALLOC(sizeof(R3D_Material)));
    model.materials[0] = R3D_GetDefaultMaterial();
    model.materialCount = 1;

    model.meshMaterials = reinterpret_cast<int*>(RL_MALLOC(sizeof(int)));
    model.meshMaterials[0] = 0;

    model.aabb = mesh.aabb;
    
    return model;
}