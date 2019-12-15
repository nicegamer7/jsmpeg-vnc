#ifndef STREAM_SERVER_H
#define STREAM_SERVER_H

#include "libwebsockets/include/libwebsockets.h"

typedef struct stream_frame_t {
    int display_number;
    int cursor_x;
    int cursor_y;
    char data[1024 * 1024];
} stream_frame_t;

typedef struct stream_server_t {
	struct lws_context *context;
	struct stream_frame_t *frame;
	int frame_size;
    char *data;
    int port;
    int active_connections;
    char *address;
	char *password;
} stream_server_t;

stream_server_t *stream_server_create(int port, char *password);
void stream_server_destroy(stream_server_t *self);
void stream_server_broadcast(stream_server_t *self, void *data, int data_size, int display_number, int cursor_x, int cursor_y);
void stream_server_update(stream_server_t *self);
void stream_server_idle(stream_server_t *self);

#endif
