#include  "test.h"

#define NUM_BARS 20  // Chỉ lấy 20 cột FFT

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
