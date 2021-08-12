#ifndef ENCODER_H
#define ENCODER_H

#include <stdbool.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

typedef struct {
    AVCodec *codec;
    AVCodecContext *codec_context;
    AVFrame *frame;

    int in_width, in_height;
    int out_width, out_height;

    struct SwsContext *sws;

    void *data;
    int data_size;
    int64_t next_pts;

    AVFormatContext *format_context;
    AVCodec *video_codec;
    AVStream *stream;


} encoder_t;

encoder_t *encoder_create(int in_width, int in_height, int out_width, int out_height, int bitrate, int buffer_size, int gop);
void encoder_destroy(encoder_t *self);
void encoder_write(encoder_t *self, void *data, int *size);
void encoder_release(encoder_t *self);
bool encoder_encode(encoder_t *self, void *rgb_pixels);

#endif
