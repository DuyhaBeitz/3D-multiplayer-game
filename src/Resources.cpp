#include "Resources.hpp"
#include <cstring>

// R3D_Model AnimatedModelAlias::CreateAlias(R3D_Model source) {
//     R3D_Model target = { 0 };

//     // Share (shallow copy) mesh and material data
//     target.meshes = source.meshes;
//     target.materials = source.materials;
//     target.meshMaterials = source.meshMaterials;
    
//     // Copy basic transform and counts
//     target.meshCount = source.meshCount;
//     target.materialCount = source.materialCount;
//     target.aabb = source.aabb;
//     target.boneCount = source.boneCount;

//     // Deep copy bones and bind pose (if any)
//     target.boneCount = source.boneCount;
//     if (source.bones && source.boneCount > 0) {
//         target.bones = (BoneInfo *)RL_CALLOC(source.boneCount, sizeof(BoneInfo));
//         memcpy(target.bones, source.bones, source.boneCount * sizeof(BoneInfo));

//         target.boneOffsets = (Matrix *)RL_CALLOC(source.boneCount, sizeof(Matrix));
//         memcpy(target.boneOffsets, source.boneOffsets, source.boneCount * sizeof(Matrix));
//     }
//     else {
//         target.bones = NULL;
//         target.boneOffsets = NULL;
//     }
//     return target;
// }

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