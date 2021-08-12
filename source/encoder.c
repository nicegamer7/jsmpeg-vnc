#include <stdio.h>
#include <stdlib.h>
#include "encoder.h"

static int encoder_write_packet(encoder_t *self, uint8_t *buf, int buf_size) {
    self->data_size = buf_size;
    return buf_size;
}

encoder_t *encoder_create(int in_width, int in_height, int out_width, int out_height, int bitrate, int buffer_size, int gop) {
    encoder_t *self = (encoder_t *) malloc(sizeof(encoder_t));
    memset(self, 0, sizeof(encoder_t));

    self->in_width = in_width;
    self->in_height = in_height;
    self->out_width = (out_width == 0) ? in_width : out_width;
    self->out_height = (out_height == 0) ? in_height : out_height;

    // Estimate bitrate
    if (bitrate == 0) {
        bitrate = self->out_width * 2000;
    }

    self->sws = sws_getContext(
            self->in_width, self->in_height, AV_PIX_FMT_RGB32,
            self->out_width, self->out_height, AV_PIX_FMT_YUV420P,
            SWS_FAST_BILINEAR, 0, 0, 0
    );
    self->data = av_malloc(buffer_size);

    self->format_context = avformat_alloc_context();
    self->format_context->oformat = av_guess_format("mpegts", NULL, NULL);

    self->format_context->pb = avio_alloc_context(self->data, buffer_size, 1, self, NULL, &encoder_write_packet, NULL);
    if (!self->format_context->pb) {
        exit(1);
    }

    self->video_codec = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);
    if (!self->video_codec) {
        exit(1);
    }

    self->stream = avformat_new_stream(self->format_context, NULL);
    if (!self->stream) {
        exit(1);
    }

    self->stream->time_base = (AVRational) {1, 60};
    self->stream->id = self->format_context->nb_streams - 1;
    self->codec_context = avcodec_alloc_context3(self->video_codec);
    self->codec_context->thread_count = 1;
    if (!self->codec_context) {
        exit(1);
    }

    self->codec_context->bit_rate = bitrate;
    self->codec_context->codec_id = AV_CODEC_ID_MPEG1VIDEO;
    self->codec_context->dct_algo = FF_DCT_FASTINT;
    self->codec_context->width = self->out_width;
    self->codec_context->height = self->out_height;
    self->codec_context->time_base = self->stream->time_base;
    self->codec_context->max_b_frames = 0;
    self->codec_context->gop_size = gop;
    self->codec_context->pix_fmt = AV_PIX_FMT_YUV420P;

    if (avcodec_open2(self->codec_context, self->video_codec, NULL) < 0) {
        exit(1);
    }

    self->frame = av_frame_alloc();
    if (!self->frame) {
        exit(1);
    }

    self->frame->format = self->codec_context->pix_fmt;
    self->frame->width = self->codec_context->width;
    self->frame->height = self->codec_context->height;

    if (av_frame_get_buffer(self->frame, 32) < 0) {
        exit(1);
    }

    if (avcodec_parameters_from_context(self->stream->codecpar, self->codec_context) < 0) {
        exit(1);
    }

    if (avformat_write_header(self->format_context, NULL) < 0) {
        exit(1);
    }

    av_log_set_level(AV_LOG_QUIET);

    return self;
}

void encoder_destroy(encoder_t *self) {
    if (self != NULL) {
        sws_freeContext(self->sws);
        avcodec_free_context(&self->codec_context);
        av_frame_free(&self->frame);
        av_free(self->data);

        free(self);
    }
}

bool encoder_encode(encoder_t *self, void *rgb_pixels) {
    AVPacket packet = {0};
    int success;

    if (av_frame_make_writable(self->frame) == 0) {
        uint8_t *in_data[1] = {(uint8_t *) rgb_pixels};
        int in_linesize[1] = {self->in_width * 4};
        sws_scale(self->sws, in_data, in_linesize, 0, self->in_height, self->frame->data, self->frame->linesize);
        self->frame->pts = self->next_pts++;

        av_init_packet(&packet);

        if (avcodec_send_frame(self->codec_context, self->frame) == 0 && avcodec_receive_packet(self->codec_context, &packet) == 0) {
            av_packet_rescale_ts(&packet, self->codec_context->time_base, self->stream->time_base);

            return av_interleaved_write_frame(self->format_context, &packet) == 0;
        }
    }

    return false;
}
