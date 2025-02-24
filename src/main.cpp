#include <algorithm>
#include <raylib.h>
#include <fftw3.h>
#include <vector>
#include <queue>
#include <fstream>
#include <cmath>
#include <iostream>

#define SAMPLE_RATE 44100
#define FRAME_SIZE_FIRST 88220
#define FRAME_SIZE 88220
#define NUM_BARS 100
#define SPF (FRAME_SIZE / NUM_BARS)
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define NUM_FRAMES 60

void print_frame(const std::vector<float>& frame)
{
    for (const auto x : frame)
        std::cout << x << " ";
    std::cout << std::endl;
}

std::ifstream pcm_file;
std::queue<std::vector<int16_t>> pcm_queue;
std::vector<std::vector<float>> fft_frames;
int num_audio_frames = 0;

bool load_pcm_file(const std::string& filename)
{
    pcm_file.open(filename, std::ios::binary);
    return pcm_file.is_open();
}

void read_pcm_frames()
{
    bool first_frame = true;
    while (!pcm_file.eof())
    {
        int frame_size = first_frame ? FRAME_SIZE_FIRST : FRAME_SIZE;
        std::vector<int16_t> buffer(frame_size, 0);

        if (!pcm_file.read(reinterpret_cast<char*>(buffer.data()), frame_size * sizeof(int16_t)))
            break;

        pcm_queue.push(buffer);
        first_frame = false;
    }
    num_audio_frames = pcm_queue.size();
    std::cout << "Đọc được " << num_audio_frames << " frame PCM.\n";
}

double temp = 0;

std::vector<float> compute_fft(const std::vector<int16_t>& buffer)
{
    const int size = buffer.size();
    std::vector fft_magnitudes(NUM_BARS, 0.0f);


    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * size);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * size);
    fftw_plan plan = fftw_plan_dft_1d(size, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for (int i = 0; i < size; i++)
    {
        in[i][0] = static_cast<double>(buffer[i]) / 32768.0;
        in[i][1] = 0.0;
    }

    fftw_execute(plan);

    for (int i = 0; i < NUM_BARS; i++)
    {
        double sum = 0.0;
        for (int j = 0; j < SPF; j++)
        {
            const int index = i * SPF + j;
            sum += sqrt(out[index][0] * out[index][0] + out[index][1] * out[index][1]);
        }
        fft_magnitudes[i] = sqrt(sum) / 1000;
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    return fft_magnitudes;
}

void process_audio()
{
    while (!pcm_queue.empty())
    {
        std::vector<int16_t> frame = pcm_queue.front();
        pcm_queue.pop();
        fft_frames.push_back(compute_fft(frame));
    }
}

void interpolate_fft(const int frame, std::vector<float>& output)
{
    if (frame % SPF == 0)
        return output = fft_frames[frame / SPF], void();
    const int left_idx = frame / SPF;
    const int right_idx = left_idx + 1;
    const float t = static_cast<float>(frame) / static_cast<float>(SPF);

    for (int i = 0; i < NUM_BARS; i++)
        output[i] = fft_frames[left_idx][i] + (fft_frames[right_idx][i] - fft_frames[left_idx][i]) * t;
}

void draw_fft(const std::vector<float>& fft_magnitudes)
{
    for (int i = 0; i < NUM_BARS; i++)
    {
        const float x = static_cast<float>(i) / NUM_BARS * SCREEN_WIDTH;
        constexpr float width = SCREEN_WIDTH / NUM_BARS - 5;
        const float height = SCREEN_HEIGHT * fft_magnitudes[i];
        const auto color = Color{255, static_cast<unsigned char>(fmin(fft_magnitudes[i] * 2, 255)), 0, 255};
        DrawRectangle(x, SCREEN_HEIGHT - height, width, height, color);
    }
}

int main(int argc, char** argv)
{
    if (argc < 2 || !load_pcm_file(argv[1]))
    {
        std::cerr << "Không mở được file PCM!" << std::endl;
        return -1;
    }

    read_pcm_frames();
    process_audio();
    if (num_audio_frames < 2)
    {
        std::cerr << "File quá ngắn!" << std::endl;
        return -1;
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "FFT Visualization");
    SetTargetFPS(60);

    int frame = 0;
    std::vector interpolated_fft(NUM_BARS, 0.0f);

    print_frame(fft_frames[0]);
    print_frame(fft_frames[1]);

    std::cout << "HO" << temp / (num_audio_frames * SPF) << std::endl;
    //
    // interpolate_fft(1, interpolated_fft);
    // print_frame(interpolated_fft);
    // // draw_fft(interpolated_fft);
    std::cerr << temp << std::endl;
    while (!WindowShouldClose())
    {
        interpolate_fft(frame, interpolated_fft);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        draw_fft(interpolated_fft);
        EndDrawing();

        frame = (frame + 1) % (num_audio_frames * NUM_FRAMES);
    }

    CloseWindow();
    return 0;
}
