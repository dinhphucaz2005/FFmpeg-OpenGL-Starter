#include <iostream>
#include <vector>
#include <complex>
#include <opencv2/opencv.hpp>

#include "test.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 400
#define NUM_BARS 20  // Số lượng cột FFT

void renderFFTVisualization(const std::vector<std::complex<double>>& fftData) {
    int n = fftData.size() / 2;  // Chỉ vẽ nửa phổ FFT
    int step = n / NUM_BARS;     // Lấy mẫu cách đều để giảm số cột
    cv::Mat img(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3, cv::Scalar(0, 0, 0));  // Ảnh nền đen

    for (int i = 0; i < NUM_BARS; i++) {
        // Tọa độ X trên ảnh
        int x1 = i * (WINDOW_WIDTH / NUM_BARS);
        int x2 = x1 + (WINDOW_WIDTH / NUM_BARS) * 0.8;

        // Tính biên độ FFT (log-scale)
        float magnitude = std::abs(fftData[i * step]) / n;
        magnitude = pow(magnitude, 0.6);
        int barHeight = log10(1 + magnitude * 200) * 40;  // Điều chỉnh chiều cao
        barHeight = std::min(barHeight, WINDOW_HEIGHT - 10); // Giới hạn chiều cao

        // Màu sắc thay đổi theo biên độ
        cv::Scalar color(50 + barHeight, 255 - barHeight, 150 + barHeight / 2);

        // Vẽ hình chữ nhật đại diện cho FFT
        cv::rectangle(img, cv::Point(x1, WINDOW_HEIGHT - 10),
                      cv::Point(x2, WINDOW_HEIGHT - 10 - barHeight), color, cv::FILLED);
    }

    // Hiển thị ảnh FFT
    cv::imshow("Audio Visualization", img);
    cv::waitKey(1);
}

int main(const int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <audio_file>" << std::endl;
        return -1;
    }

    // Khởi tạo FFmpeg
    avformat_network_init();

    AVFormatContext* formatContext = avformat_alloc_context();
    if (avformat_open_input(&formatContext, argv[1], nullptr, nullptr) != 0) {
        std::cerr << "Could not open audio file." << std::endl;
        return -1;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cerr << "Could not find stream information." << std::endl;
        return -1;
    }

    const AVCodec* codec = nullptr;
    AVCodecContext* codecContext = nullptr;
    int audioStreamIndex = -1;

    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            codec = avcodec_find_decoder(formatContext->streams[i]->codecpar->codec_id);
            codecContext = avcodec_alloc_context3(codec);
            avcodec_parameters_to_context(codecContext, formatContext->streams[i]->codecpar);
            avcodec_open2(codecContext, codec, nullptr);
            break;
        }
    }

    if (audioStreamIndex == -1) {
        std::cerr << "Could not find audio stream." << std::endl;
        return -1;
    }

    // Khởi tạo SwrContext để chuyển đổi âm thanh
    SwrContext* swrContext = swr_alloc();
    if (!swrContext) {
        std::cerr << "Could not allocate SwrContext" << std::endl;
        return -1;
    }

    AVChannelLayout out_ch_layout;
    AVChannelLayout in_ch_layout = codecContext->ch_layout;
    av_channel_layout_default(&out_ch_layout, 1);  // Chuyển đổi sang MONO

    if (swr_alloc_set_opts2(&swrContext, &out_ch_layout, AV_SAMPLE_FMT_U8, codecContext->sample_rate,
                            &in_ch_layout, codecContext->sample_fmt, codecContext->sample_rate, 0, nullptr) < 0) {
        std::cerr << "Failed to set SwrContext options" << std::endl;
        swr_free(&swrContext);
        return -1;
    }

    if (swr_init(swrContext) < 0) {
        std::cerr << "Could not initialize SwrContext" << std::endl;
        swr_free(&swrContext);
        return -1;
    }

    av_channel_layout_uninit(&out_ch_layout);

    uint8_t* outputBuffer = nullptr;
    int outputBufferSize = av_samples_get_buffer_size(nullptr, 1, codecContext->frame_size, AV_SAMPLE_FMT_U8, 1);
    av_samples_alloc(&outputBuffer, nullptr, 1, codecContext->frame_size, AV_SAMPLE_FMT_U8, 1);

    AVPacket packet;
    AVFrame* frame = av_frame_alloc();

    while (av_read_frame(formatContext, &packet) >= 0) {
        if (packet.stream_index == audioStreamIndex) {
            avcodec_send_packet(codecContext, &packet);

            while (avcodec_receive_frame(codecContext, frame) >= 0) {
                // Chuyển đổi âm thanh sang Mono, U8
                int outputSamples = swr_convert(swrContext, &outputBuffer, outputBufferSize,
                                                (const uint8_t**)frame->data, frame->nb_samples);

                if (outputSamples <= 0) continue;

                int dataSize = av_samples_get_buffer_size(nullptr, 1, outputSamples, AV_SAMPLE_FMT_U8, 1);

                std::vector<std::complex<double>> fftData(dataSize);
                for (int i = 0; i < dataSize; i++) {
                    fftData[i] = std::complex<double>(outputBuffer[i] / 255.0, 0);
                }

                getSmoothedFFT(fftData);  // Chạy thuật toán FFT
                renderFFTVisualization(fftData);  // Vẽ phổ FFT bằng OpenCV
            }
        }
        av_packet_unref(&packet);
    }

    // Dọn dẹp bộ nhớ
    av_frame_free(&frame);
    av_freep(&outputBuffer);
    swr_free(&swrContext);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);

    return 0;
}
