#ifndef APP_H
#define APP_H

#include "encoder.h"
#include "message_server.h"
#include "stream_server.h"
#include "grabber.h"
#include "input.h"

typedef struct {
	encoder_t *encoder;
	grabber_t *grabber;
	stream_server_t *stream_server;
	message_server_t *message_server;
	input_t *input;

	int buffer_size;
	int gop;

    pthread_mutex_t mutex_streaming;
	pthread_mutex_t mutex_input;

	int display_number;
	int bit_rate;
 } app_t;

app_t *app_create(int port, int display_number, int bit_rate, int allow_input, char *password, int buffer_size, int gop);
void app_destroy(app_t *self);
void app_run(app_t *self, int target_fps);

void app_on_connect(app_t *self, struct lws *socket);
void app_on_close(app_t *self, struct lws *socket);
void app_on_input(app_t *self, void *data);
void app_on_change_display(app_t *self, int value);
void app_on_paste(app_t *self, char *contents);
void app_on_copy(app_t *self, char **out_clipboard);
void app_on_key_down(app_t *self, int code);
void app_on_key_up(app_t *self, int code);
void app_on_mouse_move(app_t *self, int x, int y);
void app_on_mouse_left_down(app_t *self);
void app_on_mouse_left_up(app_t *self);
void app_on_mouse_right_down(app_t *self);
void app_on_mouse_right_up(app_t *self);
void app_on_mouse_middle_down(app_t *self);
void app_on_mouse_middle_up(app_t *self);
void app_on_mouse_scroll(app_t *self, int amount);
void app_on_upload_file(app_t *self, char *filename, int contents_size, char *contents);
void app_on_get_cursor_position(app_t *self, int *x, int *y);

#endif
