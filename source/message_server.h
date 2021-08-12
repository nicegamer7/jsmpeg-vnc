#ifndef MESSAGE_SERVER_H
#define MESSAGE_SERVER_H

#include <pthread.h>
#include "libwebsockets/include/libwebsockets.h"

typedef struct message_server_t {
    struct lws_context *context;
    void *user;
    char *password;

    pthread_t thread;

    void (*on_mouse_move)(void *self, double x, double y);
    void (*on_mouse_left_down)(void *self);
    void (*on_mouse_left_up)(void *self);
    void (*on_mouse_right_down)(void *self);
    void (*on_mouse_right_up)(void *self);
    void (*on_mouse_middle_down)(void *self);
    void (*on_mouse_middle_up)(void *self);
    void (*on_mouse_scroll)(void *self, int amount);
    void (*on_key_down)(void *self, int code);
    void (*on_key_up)(void *self, int code);
    void (*on_paste)(void *self, char *contents);
    void (*on_copy)(void *self, char **out_clipboard);
    void (*on_change_display)(void *self, int value);
    void (*on_upload_file)(void *self, char *filename, int contents_size, char *contents);
} message_server_t;

message_server_t *message_server_create(int port, char *password);
void message_server_destroy(message_server_t *self);

#endif
