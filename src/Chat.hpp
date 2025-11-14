#pragma once

#include <raylib.h>
#include <deque>
#include <cstring>
#include "Constants.hpp"
#include "Resources.hpp"

struct ChatMessage {
    char name[max_string_len] = {};
    char text[max_string_len] = {};
};

class Chat {
private:
    std::deque<ChatMessage> m_messages;
    float font_size = 64;
    float spacing = 1.0;

public:
    Chat() = default;

    void AddMessage(const ChatMessage& text) {
        if (m_messages.size() == max_chat_messages) {
            m_messages.pop_front();
        }
        m_messages.push_back(text);
    }

    Vector2 Measure(const char* text) {
        return MeasureTextEx(
            Resources::Get().FontFromKey(R_FONT_DEFAULT),
            text,
            font_size,
            spacing
        );
    }

    void Draw() {
        if (m_messages.size() > 0) {
            float max_width;
            for (auto& message : m_messages) {
                max_width = fmax(max_width, Measure(message.name).x);
                max_width = fmax(max_width, Measure(message.text).x);                              
            }
            DrawRectangle(0, 0, max_width, font_size*m_messages.size()*2, Fade(BLACK, 0.5));

            for (int i = 0; i < m_messages.size(); i++) {
                int j = i*2;
                Vector2 name_pos = Vector2{0, j*font_size};
                Vector2 text_pos = Vector2{0, (j+1)*font_size};

                DrawTextEx(
                    Resources::Get().FontFromKey(R_FONT_DEFAULT),
                    m_messages[i].name,
                    name_pos,
                    font_size,
                    spacing, RED
                );
                DrawTextEx(
                    Resources::Get().FontFromKey(R_FONT_DEFAULT),
                    m_messages[i].text,
                    text_pos,
                    font_size,
                    spacing, GREEN
                );
            }
        }
    }
};