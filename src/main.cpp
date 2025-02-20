#include <cstdio>

extern "C" {
    #include <libavcodec/avcodec.h>
}

int main(const int argc, const char **argv) {
    printf("Hello, FFmpeg!\n");
    printf("%d\n", avcodec_version());
    return 0;
}
