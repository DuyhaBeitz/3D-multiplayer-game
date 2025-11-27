#pragma once

#include <raylib.h>
#include <cstring>
#include <string>
#include <vector>

/*
If there's enough space for a new word, it's added
Otherwise the line is pushed to out_lines, and a new current_line starts with this word
*/
void TryAddWord(std::string& current_line,
                std::string& word,
                float max_width,
                Font font,
                float font_size,
                float spacing,
                std::vector<std::string>& out_lines)
{
    if (word.empty()) return;

    if (current_line.empty()) {
        current_line = word;
    }
    else {
        std::string test = current_line + " " + word;
        Vector2 size = MeasureTextEx(font, test.c_str(), font_size, spacing);

        if (size.x > max_width) {
            out_lines.push_back(current_line);
            current_line = word;
        }
        else {
            current_line = test;
        }
    }

    word.clear();
}

std::vector<std::string> WrapText(const char* text,
                                  float max_width,
                                  Font font,
                                  float font_size,
                                  float spacing)
{
    std::vector<std::string> lines;
    std::string current_line;
    std::string word;

    const char* p = text;

    while (*p) {
        char c = *p;

        if (c != ' ' && c != '\n' && c != '\t') {
            word.push_back(c);
        }
        else {
            TryAddWord(current_line, word, max_width, font, font_size, spacing, lines);

            if (c == '\n') {
                lines.push_back(current_line);
                current_line.clear();
            }
        }

        p++;
    }

    TryAddWord(current_line, word, max_width, font, font_size, spacing, lines);

    if (!current_line.empty())
        lines.push_back(current_line);

    return lines;
}