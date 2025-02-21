#include  "test.h"

#define NUM_BARS 20  // Chỉ lấy 20 cột FFT

void fft(std::vector<std::complex<double>>& a) {
    int n = a.size();
    if (n <= 1) return;

    // Chia thành phần chẵn và lẻ
    std::vector<std::complex<double>> even(n / 2), odd(n / 2);
    for (int i = 0; i < n / 2; i++) {
        even[i] = a[i * 2];
        odd[i] = a[i * 2 + 1];
    }

    // Đệ quy FFT
    fft(even);
    fft(odd);

    // Hợp nhất kết quả
    for (int k = 0; k < n / 2; k++) {
        std::complex<double> t = std::polar(1.0, -2 * PI * k / n) * odd[k];
        a[k] = even[k] + t;
        a[k + n / 2] = even[k] - t;
    }
}

std::vector<double> getSmoothedFFT(const std::vector<std::complex<double>>& fftData) {
    int n = fftData.size() / 2;  // Chỉ lấy nửa phổ FFT
    std::vector<double> magnitudes(n);

    // Tính biên độ FFT
    for (int i = 0; i < n; i++) {
        magnitudes[i] = std::abs(fftData[i]) / n;
    }

    // Chỉ chọn 20 cột từ phổ FFT
    std::vector<double> bars(NUM_BARS, 0.0);
    int step = std::max(1, n / NUM_BARS);  // Chia thành 20 nhóm

    for (int i = 0; i < NUM_BARS; i++) {
        double sum = 0.0;
        int count = 0;

        // Trung bình nhiều giá trị để làm mượt phổ
        for (int j = i * step; j < (i + 1) * step && j < n; j++) {
            sum += magnitudes[j];
            count++;
        }
        bars[i] = (count > 0) ? (sum / count) : 0;
    }

    // Áp dụng log-scale để cân bằng
    for (int i = 0; i < NUM_BARS; i++) {
        bars[i] = log10(1 + bars[i] * 200) * 0.25;
    }

    return bars;
}
