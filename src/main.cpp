#include <string>

#include "Bar.h"
#include "define.h"
#include "extension.h"
#include "raylib.h"
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <fftw3.h>
}

void print(const Color color) {
    printf("Color(%d, %d, %d, %d)\n", color.r, color.g, color.b, color.a);
}

void drawBars(const Bar *bars, const int count, const int maxWidth, const int maxHeight, const int barSpacing) {
    const int barWidth = (maxWidth - (count - 1) * barSpacing) / count;

    for (int i = 0; i < count; i++) {
        const int x = i * (barWidth + barSpacing);
        const int height = bars[i].getPercent() * maxHeight / Bar::maxPercent;
        const int y = GetScreenHeight() - height;
        Color barColor = bars[i].getColor();
        Color glowColor = Fade(barColor, 0.4f);

        // 🌟 Hiệu ứng phát sáng
        for (int j = 4; j > 0; j--) {
            DrawRectangleRounded((Rectangle){x - j, y - j, (float)barWidth + 2*j, (float)height + 2*j}, 0.5f, 6, Fade(glowColor, 0.1f * j));
        }

        // 🎨 Vẽ gradient đẹp hơn
        DrawRectangleGradientV(x, y, barWidth, height, Fade(barColor, 0.7f), barColor);

        // 🏆 Viền mượt hơn với bo tròn
        DrawRectangleRounded((Rectangle){x, y, (float)barWidth, (float)height}, 0.4f, 6, barColor);

        // ✨ Hiệu ứng sáng bóng trên cùng
        DrawRectangleRounded((Rectangle){x + 2, y + 2, (float)barWidth - 4, (float)(height * 0.3f)}, 0.6f, 6, Fade(WHITE, 0.3f));

        // 🌊 Hiệu ứng động - tạo cảm giác nhịp sóng
        if (GetTime() - (i * 0.1f) > 0) {
            int waveOffset = (int)(sin(GetTime() * 4 + i * 0.3) * 6);
            DrawRectangleRounded((Rectangle){x, y - waveOffset, (float)barWidth, (float)height + waveOffset}, 0.4f, 6, Fade(barColor, 0.9f));
        }
    }
}



constexpr double updateInterval = 1.0 / 10.0;
constexpr float lerpSpeed = 0.1f;
Bar bars[NUM_BARS];
Bar targetBars[NUM_BARS];

void init() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "nullptr");
    SetTargetFPS(144);
    for (int i = 0; i < NUM_BARS; i++)
        bars[i] = getRandBar();
}

int main() {
    init();
    const Font font = LoadFont("assets/JetBrainsMono-Bold.ttf");


    double lastUpdateTime = 0.0;
    while (!WindowShouldClose()) {
        if (const double currentTime = GetTime(); currentTime - lastUpdateTime >= updateInterval) {
            lastUpdateTime = currentTime;
            for (int i = 0; i < NUM_BARS; i++)
                targetBars[i] = getRandBar();
        }

        for (int i = 0; i < NUM_BARS; i++)
            bars[i] = extLerp(bars[i], targetBars[i], lerpSpeed);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        std::string versionText = std::string("FFmpeg version: ") + av_version_info();
        DrawTextEx(font, versionText.c_str(), {350, 280}, 20, 2, DARKBLUE);

        drawBars(bars, NUM_BARS, WINDOW_WIDTH, WINDOW_HEIGHT, 10);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
