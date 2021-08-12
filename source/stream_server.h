#ifndef STREAM_SERVER_H
#define STREAM_SERVER_H

#include "libwebsockets/include/libwebsockets.h"

typedef struct stream_server_t {
    struct lws_context *context;
    struct lws_vhost *vhost;
    struct lws_protocols protocols[3];

    int display_number;
    int cursor_x;
    int cursor_y;

    int buffer_size;
    int port;
    int active_connections;

    char *address;
    char *password;
    char cookies[256];
} stream_server_t;

stream_server_t *stream_server_create(int port, char *password, int buffer_size);
void stream_server_destroy(stream_server_t *self);
void stream_server_broadcast(stream_server_t *self, void *data, int data_size, int display_number, int cursor_x, int cursor_y);
void stream_server_update(stream_server_t *self);
void stream_server_idle(stream_server_t *self);

#endif
