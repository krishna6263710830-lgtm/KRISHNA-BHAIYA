#include "InfoDisplay.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "image/TelegramIcon.h"
#include "imgui/stb_image.h"
#include <GLES3/gl3.h>
#include <android/log.h>
#include <vector>
#include <string>
#include <cmath>
#include <cstring>

static GLuint telegramTexture = 0;
static int telegramIconWidth = 0, telegramIconHeight = 0;

void LoadTelegramTexture() {
    if (telegramTexture != 0) return;

    int channels = 0;
    unsigned char* image_data = stbi_load_from_memory(
        telegram_icon_data,
        telegram_icon_data_len,
        &telegramIconWidth,
        &telegramIconHeight,
        &channels,
        4
    );

    if (image_data == NULL) {
        return;
    }

    glGenTextures(1, &telegramTexture);
    glBindTexture(GL_TEXTURE_2D, telegramTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, telegramIconWidth, telegramIconHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);
}












void AddAnimatedIconAndText(ImDrawList* drawList, ImFont* font, float font_size, ImVec2 pos, const char* text) {
    static const std::vector<ImVec4> palette = {
        {1.0f, 0.0f, 0.8f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f}
    };

    float time = ImGui::GetTime();
    float speed = 0.4f;
    float offset = 0.08f;

    ImVec2 current_pos = pos;

    if (telegramTexture != 0) {
        float icon_size = font_size * 1.1f;
        ImVec2 icon_pos = ImVec2(current_pos.x, current_pos.y + (font_size - icon_size) / 2.0f);

        drawList->AddImage(
            (ImTextureID)(intptr_t)telegramTexture,
            icon_pos,
            ImVec2(icon_pos.x + icon_size, icon_pos.y + icon_size)
        );

        current_pos.x += icon_size + (font_size * 0.2f);
    }

    int text_len = strlen(text);
    for (int i = 0; i < text_len; ++i) {
        float progress = fmodf(time * speed + i * offset, 1.0f);
        float blend_factor = (sinf(progress * 2.0f * 3.14159f) + 1.0f) / 2.0f;
        ImVec4 char_color_vec = ImLerp(palette[0], palette[1], blend_factor);
        ImU32 char_color = ImGui::GetColorU32(char_color_vec);

        char single_char_str[2] = { text[i], '\0' };
        
        drawList->AddText(font, font_size, ImVec2(current_pos.x + 1, current_pos.y + 1), IM_COL32(0, 0, 0, 200), single_char_str);
        drawList->AddText(font, font_size, current_pos, char_color, single_char_str);

        ImVec2 char_size = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, single_char_str);
        current_pos.x += char_size.x;
    }
}











void InitializeInfoGraphics() {
    LoadTelegramTexture();
}

void RenderInfoDisplay() {
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    float screenWidth = io.DisplaySize.x;
    float screenHeight = io.DisplaySize.y;

    ImFont* font = ImGui::GetFont();
    if (!font || !font->IsLoaded()) return;

    float fontSize = screenHeight / 25.0f;
    ImVec2 position = { 40.0f, screenHeight - 150.0f };

    AddAnimatedIconAndText(drawList, font, fontSize, position, "GODXOPGOD");
}
