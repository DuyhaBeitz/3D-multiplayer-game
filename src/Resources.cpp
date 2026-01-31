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