//
// Created by nd-phuc on 2/23/25.
//

#ifndef PCM_H
#define PCM_H


extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

#include <iostream>
#include <fstream>

inline int getPCM(const char* input_file, const char* output_pcm)
{
    avformat_network_init();
    AVFormatContext* formatCtx = nullptr;

    if (avformat_open_input(&formatCtx, input_file, nullptr, nullptr) < 0)
    {
        printf("Không thể mở file: %s", input_file);
        return -1;
    }

    if (avformat_find_stream_info(formatCtx, nullptr) < 0)
    {
        std::cerr << "Không thể lấy thông tin stream\n";
        return -1;
    }

    // Tìm stream âm thanh
    int audioStreamIndex = -1;
    for (unsigned i = 0; i < formatCtx->nb_streams; i++)
    {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIndex = i;
            break;
        }
    }

    if (audioStreamIndex == -1)
    {
        std::cerr << "Không tìm thấy stream âm thanh\n";
        return -1;
    }

    // Mở codec
    const AVCodecParameters* codec_parameters = formatCtx->streams[audioStreamIndex]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codec_parameters->codec_id);
    AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx, codec_parameters);
    avcodec_open2(codecCtx, codec, nullptr);

    SwrContext* swrCtx = nullptr;
    AVChannelLayout ch_layout;
    av_channel_layout_default(&ch_layout, 1);


    int ret = swr_alloc_set_opts2(
        &swrCtx,
        &ch_layout, AV_SAMPLE_FMT_S16, 44100, // Định dạng đầu ra
        &codecCtx->ch_layout, codecCtx->sample_fmt, codecCtx->sample_rate, // Định dạng đầu vào
        0, nullptr);

    if (ret < 0 || !swrCtx)
    {
        std::cerr << "Lỗi khởi tạo SwrContext\n";
        return -1;
    }

    if (swr_init(swrCtx) < 0)
    {
        std::cerr << "Lỗi khi khởi tạo SwrContext\n";
        swr_free(&swrCtx);
        return -1;
    }

    // Mở file PCM để lưu kết quả
    std::ofstream pcmFile(output_pcm, std::ios::binary);

    // Đọc và giải mã từng frame
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    while (av_read_frame(formatCtx, packet) >= 0)
    {
        if (packet->stream_index == audioStreamIndex)
        {
            if (avcodec_send_packet(codecCtx, packet) == 0)
            {
                while (avcodec_receive_frame(codecCtx, frame) == 0)
                {
                    uint8_t* outputBuffer;
                    av_samples_alloc(&outputBuffer, nullptr, 1, frame->nb_samples, AV_SAMPLE_FMT_S16, 0);

                    int samplesConverted = swr_convert(swrCtx, &outputBuffer, frame->nb_samples,
                                                       frame->data, frame->nb_samples);

                    pcmFile.write(reinterpret_cast<char*>(outputBuffer), samplesConverted * 2);
                    av_freep(&outputBuffer);
                }
            }
        }
        av_packet_unref(packet);
    }

    pcmFile.close();
    av_frame_free(&frame);
    av_packet_free(&packet);
    swr_free(&swrCtx);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);
    avformat_network_deinit();

    std::cout << "Đã trích xuất PCM thành công: " << output_pcm << std::endl;
    return 0;
}


#endif //PCM_H
