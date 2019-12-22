#include <stdio.h>
#include <stdlib.h>

#include "app.h"
#include "os.h"

app_t *app_create(int port, int display_number, int bit_rate, int allow_input, char *password, int buffer_size, int gop)
{
	app_t *self = (app_t *)malloc(sizeof(app_t));
	memset(self, 0, sizeof(app_t));

    os_set_current_dir();

	self->bit_rate = bit_rate;
	self->display_number = display_number;

	if (!os_is_display(self->display_number)) {
        printf("Invalid display\n");
        exit(1);
    }

    self->buffer_size = buffer_size;
    self->gop = gop;

    self->input = input_create(self->display_number);
    self->grabber = grabber_create(self->display_number);
    self->encoder = encoder_create(self->grabber->width, self->grabber->height, 0, 0, self->bit_rate, buffer_size, gop);

    self->stream_server = stream_server_create(port, password, buffer_size);

    self->message_server = message_server_create(port + 1, password);
    self->message_server->user = self;
    self->message_server->on_change_display = app_on_change_display;

    if (allow_input) {
        self->message_server->on_paste = app_on_paste;
        self->message_server->on_copy = app_on_copy;
        self->message_server->on_key_down = app_on_key_down;
        self->message_server->on_key_up = app_on_key_up;
        self->message_server->on_mouse_move = app_on_mouse_move;
        self->message_server->on_mouse_left_down = app_on_mouse_left_down;
        self->message_server->on_mouse_left_up = app_on_mouse_left_up;
        self->message_server->on_mouse_right_down = app_on_mouse_right_down;
        self->message_server->on_mouse_right_up = app_on_mouse_right_up;
        self->message_server->on_mouse_middle_down = app_on_mouse_middle_down;
        self->message_server->on_mouse_middle_up = app_on_mouse_middle_up;
        self->message_server->on_mouse_scroll = app_on_mouse_scroll;
        self->message_server->on_upload_file = app_on_upload_file;
    }

	pthread_mutex_init(&self->mutex_streaming, NULL);
	pthread_mutex_init(&self->mutex_input, NULL);

	return self;
}

void app_destroy(app_t *self) {

	if (self != NULL) {
        encoder_destroy(self->encoder);
        grabber_destroy(self->grabber);
        stream_server_destroy(self->stream_server);
        message_server_destroy(self->message_server);
        input_destroy(self->input);

        free(self);
    }
}

void app_on_change_display(app_t *self, int value) {

    if (value == self->display_number) {
        return;
    }

    printf("Changing display: %d\n", value);

    if (os_is_display(value)) {

        pthread_mutex_lock(&self->mutex_input);
        pthread_mutex_lock(&self->mutex_streaming);

        input_destroy(self->input);
        grabber_destroy(self->grabber);
        encoder_destroy(self->encoder);

        self->display_number = value;
        self->input = input_create(self->display_number);
        self->grabber = grabber_create(self->display_number);
        self->encoder = encoder_create(self->grabber->width, self->grabber->height, 0, 0, self->bit_rate, self->buffer_size, self->gop);

        pthread_mutex_unlock(&self->mutex_input);
        pthread_mutex_unlock(&self->mutex_streaming);
    }
}

void app_on_paste(app_t *self, char *contents)
{
    os_set_clipboard(contents, self->display_number);
}

void app_on_copy(app_t *self, char **out_clipboard)
{
    *out_clipboard = os_get_clipboard(self->display_number);
}

void app_on_key_down(app_t *self, int code)
{
    pthread_mutex_lock(&self->mutex_input);
    input_key_press(self->input, code, true);
    pthread_mutex_unlock(&self->mutex_input);
}

void app_on_key_up(app_t *self, int code)
{
    pthread_mutex_lock(&self->mutex_input);
    input_key_press(self->input, code, false);
    pthread_mutex_unlock(&self->mutex_input);
}

void app_on_mouse_move(app_t *self, int x, int y)
{
    pthread_mutex_lock(&self->mutex_input);
    input_mouse_move(self->input, x, y);
    pthread_mutex_unlock(&self->mutex_input);
}

void app_on_mouse_left_down(app_t *self)
{
    pthread_mutex_lock(&self->mutex_input);
    input_mouse_left_button(self->input, true);
    pthread_mutex_unlock(&self->mutex_input);
}

void app_on_mouse_left_up(app_t *self)
{
    pthread_mutex_lock(&self->mutex_input);
    input_mouse_left_button(self->input, false);
    pthread_mutex_unlock(&self->mutex_input);
}

void app_on_mouse_right_down(app_t *self)
{
    pthread_mutex_lock(&self->mutex_input);
    input_mouse_right_button(self->input, true);
    pthread_mutex_unlock(&self->mutex_input);
}

void app_on_mouse_right_up(app_t *self)
{
    pthread_mutex_lock(&self->mutex_input);
    input_mouse_right_button(self->input, false);
    pthread_mutex_unlock(&self->mutex_input);
}

void app_on_mouse_middle_down(app_t *self)
{
    pthread_mutex_lock(&self->mutex_input);
    input_mouse_middle_button(self->input, true);
    pthread_mutex_unlock(&self->mutex_input);
}

void app_on_mouse_middle_up(app_t *self)
{
    pthread_mutex_lock(&self->mutex_input);
    input_mouse_middle_button(self->input, false);
    pthread_mutex_unlock(&self->mutex_input);
}

void app_on_mouse_scroll(app_t *self, int amount)
{
    pthread_mutex_lock(&self->mutex_input);
    input_mouse_scroll(self->input, amount);
    pthread_mutex_unlock(&self->mutex_input);
}

void app_on_upload_file(app_t *self, char *filename, int contents_size, char *contents)
{
    os_save_upload(contents, contents_size, filename);
}

#define timer_measure(measure, average) { \
    double start = os_get_time();         \
    measure;                              \
    double stop = os_get_time();          \
    average -= average / 30;              \
    average += (stop - start) / 30;       \
}

void app_run(app_t *self, int target_fps)
{
    double frame_interval = 0;
    int fps;
    double grab_time, encode_time, frame_time;
    int64_t frames = 0;
    int x, y;

    while (true) {

        while (self->stream_server->active_connections == 0) {
            stream_server_idle(self->stream_server);
        }

        timer_measure(

            if (frame_interval > 0) {
                os_sleep(frame_interval);
            }

            pthread_mutex_lock(&self->mutex_streaming);

            input_get_cursor_position(self->input, &x, &y);

            timer_measure(grabber_grab(self->grabber), grab_time);
            timer_measure(encoder_encode(self->encoder, self->grabber->buffer), encode_time);

            stream_server_broadcast(self->stream_server, self->encoder->data, self->encoder->data_size, self->display_number, x, y);
            stream_server_update(self->stream_server);

            pthread_mutex_unlock(&self->mutex_streaming);

            if ((frames++ > target_fps) && (frames % 4 == 0)) {

                fps = (int) (1000.0f / frame_time);

                if (fps > target_fps) {
                    frame_interval += 0.50f;
                }
                if (fps < target_fps) {
                    frame_interval -= 0.50f;
                }
                if (frame_interval < 0) {
                    frame_interval = 0;
                }

                printf("FPS: %2d (grabbing: %.2f ms) (encoding %.2f ms)\r", fps, grab_time, encode_time);

                fflush(stdout);
            },

            frame_time
        );

    }
}
