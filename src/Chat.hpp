#pragma once

#include <raylib.h>
#include <deque>

#include <cmath>
#include "Constants.hpp"
#include "Resources.hpp"

#include "TextHelper.hpp"

struct ChatMessage {
    char name[max_player_name_len] = {};
    char text[max_string_len] = {};
};

class Chat {
private:
    std::deque<ChatMessage> m_messages;
    float font_size = 64;
    float spacing = 1.0;

public:
    Chat() = default;

    void AddMessage(const ChatMessage& message) {
        if (!m_messages.empty()) {
            ChatMessage& last = m_messages.back();

            if (strcmp(last.name, message.name) == 0) {
                char temp[max_string_len];
                snprintf(
                    temp,
                    max_string_len,
                    "%s\n%s",
                    last.text,
                    message.text
                );

                strncpy(last.text, temp, max_string_len);
                last.text[max_string_len - 1] = '\0';

                return;
            }
        }

        m_messages.push_back(message);

        while (m_messages.size() > max_chat_messages) {
            m_messages.pop_front();
        }
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
        if (m_messages.empty()) return;

        Font font = Resources::Get().FontFromKey(R_FONT_DEFAULT);

        const float max_width = GetScreenWidth() * 0.35f;

        float total_height = 0.0f;
        std::vector<std::vector<std::string>> wrapped_texts;
        wrapped_texts.reserve(m_messages.size());

        for (auto& msg : m_messages) {
            // name is 1 line
            total_height += font_size;

            std::vector<std::string> lines = WrapText(msg.text, max_width, font, font_size, spacing);
            total_height += lines.size() * font_size;

            wrapped_texts.push_back(std::move(lines));
        }

        Vector2 offset{GetScreenWidth() - max_width, GetScreenHeight() / 2.0f -total_height};
        DrawRectangle(offset.x, offset.y, max_width, total_height, Fade(BLACK, 0.5f));

        float y = 0.0f;
        for (int i = 0; i < m_messages.size(); i++) {
            auto& msg = m_messages[i];

            Color name_clr = GOLD;
            if (strcmp(msg.name, server_chat_name) == 0) name_clr = RED;

            DrawLineV(Vector2{0, y}+offset, Vector2{max_width, y}+offset, name_clr);
            DrawTextEx(font, msg.name, Vector2{0, y}+offset, font_size, spacing, name_clr);
            y += font_size;

            for (auto& line : wrapped_texts[i]) {
                DrawTextEx(font, line.c_str(), Vector2{0, y}+offset, font_size, spacing, LIGHTGRAY);
                y += font_size;
            }
        }
    }
};