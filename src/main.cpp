#include <raylib.h>
#include <fftw3.h>
#include <vector>
#include <fstream>
#include <cmath>
#include <iostream>
#include <regex>
#include <bits/atomic_base.h>

#include "draw.h"
#include "test.h"

#define SAMPLE_RATE 44100
#define FRAME_SIZE 44100
#define NUM_BARS 20
#define SPF 2205 // samples per frame

std::ifstream pcm_file;
std::vector<int16_t> pcm_buffer(FRAME_SIZE, 0);

void read_pcm_frame()
{
    if (pcm_file && !pcm_file.eof())
    {
        pcm_file.read(reinterpret_cast<char*>(pcm_buffer.data()), FRAME_SIZE * sizeof(int16_t));
    }
}

std::vector<int16_t> fft_magnitudes(20, 0);

void compute_fft(std::vector<int16_t>& fft_magnitudes)
{
    fftw_complex *in, *out;
    fftw_plan plan;

    in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * FRAME_SIZE);
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * FRAME_SIZE);

    // Chuyển dữ liệu PCM vào FFT input (giữ nguyên raw PCM)
    for (int i = 0; i < FRAME_SIZE; i++)
    {
        in[i][0] = pcm_buffer[i]; // Không chuẩn hóa, giữ giá trị gốc
        in[i][1] = 0.0;
    }

    plan = fftw_plan_dft_1d(FRAME_SIZE, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);

    float sum = 0;
    for (int i = 0; i < FRAME_SIZE; i++)
    {
        const float bien_do = sqrt(pow(out[i][0], 2) + pow(out[i][1], 2));
        sum += bien_do;
        if (i % SPF == 0)
        {
            fft_magnitudes[i / SPF] = sum;
            sum = 0;
        }
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
}



void draw_fft(const std::vector<int16_t>& fft_magnitudes)
{
    std::vector<u_int16_t> fft_magnitudes_copy;
    for (const auto x : fft_magnitudes)
    {
        u_int16_t magnitude = static_cast<u_int16_t>(x) + 32768;
        fft_magnitudes_copy.push_back(magnitude);
    }

    for (int i = 0; i < NUM_BARS; i++)
    {
        const float x = static_cast<float>(i) / NUM_BARS * SCREEN_WIDTH;
        float height = static_cast<float>(fft_magnitudes_copy[i]) / 65536 * SCREEN_HEIGHT;
        height = fmin(height, SCREEN_HEIGHT);
        std::cerr << height << " ";
        DrawRectangle(x, SCREEN_HEIGHT - height, SCREEN_WIDTH / NUM_BARS, height, RED);
    }
    std::cerr << std::endl;
}


bool load_pcm_file(const std::string& filename)
{
    pcm_file.open(filename, std::ios::binary);
    return pcm_file.is_open();
}

int main(int argc, char** argv)
{
    if (!load_pcm_file(argv[1]))
    {
        std::cerr << "Không mở được file PCM!" << std::endl;
        return -1;
    }


    freopen64("test.out", "w", stdout);
    while (pcm_file && !pcm_file.eof())
    {
        pcm_file.read(reinterpret_cast<char*>(pcm_buffer.data()), FRAME_SIZE * sizeof(int16_t));
        compute_fft(fft_magnitudes);
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Real-Time Audio Visualization");
    SetTargetFPS(60);


    while (!WindowShouldClose())
    {
        ClearBackground(WHITE);

        draw_fft(fft_magnitudes);

        EndDrawing();
    }

    CloseWindow();
    pcm_file.close();
    return 0;
}
