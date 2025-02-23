#include <raylib.h>
#include <fftw3.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <cmath>

#include "draw.h"

#define SAMPLE_RATE 44100
#define FFT_SIZE 1024
#define BAR_COUNT 64
#define SMOOTHING_FACTOR 0.2f


// Tính FFT
void ComputeFFT(const std::vector<int16_t>& pcmData, size_t offset, std::vector<float>& spectrum)
{
    fftw_complex *in, *out;
    fftw_plan plan;

    in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);

    for (int i = 0; i < FFT_SIZE; i++)
    {
        int index = offset + i;
        in[i][0] = index < pcmData.size() ? pcmData[index] / 32768.0f : 0;
        in[i][1] = 0;
    }

    plan = fftw_plan_dft_1d(FFT_SIZE, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);

    for (int i = 0; i < BAR_COUNT; i++)
    {
        int index = i * (FFT_SIZE / 2) / BAR_COUNT;
        float magnitude = sqrt(out[index][0] * out[index][0] + out[index][1] * out[index][1]);

        spectrum[i] = spectrum[i] * (1 - SMOOTHING_FACTOR) + magnitude * SMOOTHING_FACTOR;
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
}

std::vector<int16_t> load_pcm_file(const char* filename)
{
    std::ifstream pcmFile(filename, std::ios::binary);
    if (!pcmFile) {
        std::cerr << "Không thể mở file " << filename << std::endl;
        return {};
    }

    std::vector<int16_t> pcmData;
    int16_t sample;
    while (pcmFile.read(reinterpret_cast<char *>(&sample), sizeof(int16_t))) {
        pcmData.push_back(sample);
    }

    printf("PCM data size: %lu\n", pcmData.size());
    pcmFile.close();
    return pcmData;
}

int main(const int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s filename\n", argv[0]);
        return -1;
    }

    const std::vector<int16_t> pcmData = load_pcm_file(argv[1]);
    if (pcmData.empty()) return -1;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "FFT Spectrum - Bar");
    SetTargetFPS(FPS);
    Font myFont = LoadFontEx("assets/JetBrainsMono-Bold.ttf", 32, nullptr, 0);
    if (myFont.texture.id == 0) myFont = GetFontDefault();

    std::vector spectrum(BAR_COUNT, 0.0f);
    size_t offset = 0;

    while (!WindowShouldClose())
    {
        ComputeFFT(pcmData, offset, spectrum);

        offset += FFT_SIZE / 2;
        if (offset + FFT_SIZE >= pcmData.size()) offset = 0;

        BeginDrawing();
        ClearBackground(BLACK);

        // Vẽ dạng bar
        constexpr float bar_width = static_cast<float>(SCREEN_WIDTH) / static_cast<float>((BAR_COUNT));
        for (int i = 0; i < BAR_COUNT; i++)
        {
            const float x = i * bar_width;
            const float height = spectrum[i] * 5.0f;
            const float y = SCREEN_HEIGHT - height;

            DrawRectangle(x, y, bar_width - 2, height, BLUE);
        }

        DrawTextEx(myFont, "NGUYEN DINH PHUC", {50, 50}, 40, 2, WHITE);
        EndDrawing();
    }

    UnloadFont(myFont);
    CloseWindow();
    return 0;
}
