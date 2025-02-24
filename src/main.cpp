#include "raylib.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <fftw3.h>

#define SAMPLE_RATE 44100
#define FRAME_SIZE 735  // Số mẫu trong 1 frame (nhỏ hơn để vẽ mượt hơn)
#define NUM_BARS 40      // Số cột trong phổ FFT
#define SPF (FRAME_SIZE / NUM_BARS) // samples per frequency bin

std::ifstream pcm_file;
std::vector<int16_t> pcm_buffer(FRAME_SIZE, 0);
std::vector<double> hanning_window(FRAME_SIZE, 0);

// 📌 **Bước 1: Mở file PCM**
bool load_pcm_file(const std::string& filename)
{
    pcm_file.open(filename, std::ios::binary);
    return pcm_file.is_open();
}

// 📌 **Bước 2: Đọc 1 frame (FRAME_SIZE mẫu)**
void read_pcm_frame()
{
    if (pcm_file && !pcm_file.eof())
    {
        pcm_file.read(reinterpret_cast<char*>(pcm_buffer.data()), FRAME_SIZE * sizeof(int16_t));
    }
}

// 📌 **Bước 3: Áp dụng cửa sổ Hanning**
void apply_hanning_window(std::vector<double>& samples)
{
    for (int i = 0; i < FRAME_SIZE; ++i)
    {
        double w = 0.5 * (1 - cos(2 * M_PI * i / (FRAME_SIZE - 1)));
        samples[i] *= w;
    }
}

// 📌 **Bước 4: Chuyển đổi thành số phức để làm FFT**
std::vector<std::complex<double>> set_complex_vector(const std::vector<double>& samples)
{
    std::vector<std::complex<double>> complex_vector(FRAME_SIZE);
    for (int i = 0; i < FRAME_SIZE; ++i)
    {
        complex_vector[i] = std::complex(samples[i], 0.0);
    }
    return complex_vector;
}

// 📌 **Bước 5: Áp dụng FFT**
void apply_fft(std::vector<std::complex<double>>& complex_vector)
{
    const auto in = reinterpret_cast<fftw_complex*>(complex_vector.data());
    const auto out = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * FRAME_SIZE));

    const fftw_plan plan = fftw_plan_dft_1d(FRAME_SIZE, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);

    for (int i = 0; i < FRAME_SIZE; ++i)
        complex_vector[i] = std::complex(out[i][0], out[i][1]);

    fftw_destroy_plan(plan);
    fftw_free(out);
}

// 📌 **Bước 5: Tính phổ tần số**
void compute_spectrum(const std::vector<std::complex<double>>& fft_output, std::vector<double>& spectrum)
{
    for (int j = 0; j < FRAME_SIZE / 2; ++j)
    {
        spectrum[j] = std::abs(fft_output[j]);
    }
}

// 📌 **Bước 6: Biến đổi phổ tần**
void apply_spectrum_transform(std::vector<double>& spectrum, const double fit_factor, const double fit_factor2)
{
    for (auto& value : spectrum)
        value = log(value * fit_factor2) * fit_factor;
}


int main(const int argc, const char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return EXIT_FAILURE;
    }

    constexpr int screen_width = 1920;
    constexpr int screen_height = 1080;

    InitWindow(screen_width, screen_height, "PCM Waveform & FFT Spectrum");
    SetTargetFPS(60);
    const Font default_font = LoadFont("fonts/my_font.ttf");
    if (!load_pcm_file(argv[1]))
    {
        std::cerr << "Failed to open PCM file!\n";
        return -1;
    }


    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawTextEx(default_font, "NGUYEN DINH PHUC", {300, 50}, 180, 8, {0, 128, 128, 255});

        // Vẽ waveform
        // for (int i = 0; i < FRAME_SIZE - 1; i++)
        // {
        //     float x1 = (i / (float)FRAME_SIZE) * screenWidth;
        //     float y1 = screenHeight / 2 + pcm_float[i] * (screenHeight / 2);
        //
        //     float x2 = ((i + 1) / (float)FRAME_SIZE) * screenWidth;
        //     float y2 = screenHeight / 2 + pcm_float[i + 1] * (screenHeight / 2);
        //
        //     DrawLine(x1, y1, x2, y2, BLUE);
        // }
        read_pcm_frame(); // Đọc frame đầu tiên

        // Chuyển PCM sang float [-1,1]
        std::vector<double> pcm_float(FRAME_SIZE);
        for (int i = 0; i < FRAME_SIZE; ++i)
        {
            pcm_float[i] = pcm_buffer[i] / 32768.0;
        }

        // Áp dụng Hanning Window
        apply_hanning_window(pcm_float);

        // Chuyển đổi thành số phức và thực hiện FFT
        auto complex_vector = set_complex_vector(pcm_float);
        apply_fft(complex_vector);

        // Tính biên độ phổ
        std::vector<double> spectrum(FRAME_SIZE / 2, 0);
        compute_spectrum(complex_vector, spectrum);

        // Áp dụng hiệu chỉnh phổ
        apply_spectrum_transform(spectrum, 0.1, 50);
        // Vẽ phổ FFT
        const Color color = GetRandomValue(0, 0) == 1 ? Color({248,201,209, 255}) : Color({219, 66, 79, 255});

        for (int i = 0; i < NUM_BARS; i++)
        {
            constexpr float bar_width = screen_width / static_cast<float>(NUM_BARS);
            const float bar_height = spectrum[i] * screen_height;
            DrawRectangle(i * bar_width, screen_height - bar_height, bar_width - 10, bar_height, color);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

/*
1- Collect N_Samples Vector from sound card
2- Appli window function to N_Samples Vector (Hann aka Hanning window)
3- Set a ComplexVector ( Real = N_Samples , Img = 0)
4- Appli FFT(ComplexVector)

5- Set peak frequencies in the spectrum array
 for(j = 0; j ＜(N/2)    ; j++){
  magnitude = sqrt( ComplexVector[j].re^2+ ComplexVector[j].img^2);
  freq      =  j * SampleRate /N;
  for( i=0;i＜BUCKETS;i++){
   if( freq＞=Freq_Bin[i] && freq＜=Freq_Bin[i+1] ){
    if (magnitude ＞ Spectrum[i]){
     Spectrum[i] = magnitude;
    }
   }
  }
 }
Freq_Bin: is a distribution of audible frequencies

6- If you want appli Spectrum deformations f(x)

Spectrum[i]= f( Spectrum[i] )

Natural
f(x)=x*Fit_factor

Exponential
f(x)=log(x*Fit_factor2)*Fit_factor

Multi Peak Scale
f(x,i)=x/Peak[i]*Fit_factor

Max Peak Scale
f(x)=x/Global_Peak*Fit_factor

7- Render spectrum magnitudes (Spectrum[i])

*/
