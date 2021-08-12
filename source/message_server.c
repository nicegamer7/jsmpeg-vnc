#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef __linux__
	#include <unistd.h>
	#include <arpa/inet.h>
#endif

#include "message_server.h"

typedef struct client_t {
	struct lws *wsi;

	bool authenticated;
	char address[32];
	char password[255];

	char *receive_buffer;
	size_t receive_buffer_position;
	size_t receive_buffer_size;

	char *send_buffer;
    int send_buffer_size;
} client_t;

static int callback_websockets(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *data, size_t len);

static struct lws_protocols protocols[] = {
    {"http", lws_callback_http_dummy, 0,                0},
    {"ws",   callback_websockets,     sizeof(client_t), 256 * 256},
    {NULL, NULL,                      0,                0}
};

void *message_server_service(void *user)
{
	message_server_t *self = (message_server_t *) user;

	while (true) {
        lws_service(self->context, 1000);
	}
}

message_server_t * message_server_create(int port, char *password) {

    message_server_t *self = (message_server_t *) malloc(sizeof(message_server_t));
    memset(self, 0, sizeof(message_server_t));

    struct lws_context_creation_info info = {0};

	info.port = port;
    info.gid = -1;
    info.uid = -1;
    info.user = (void *)self;
    info.protocols = protocols;

    self->password = password;
    self->context = lws_create_context(&info);

    if (!self->context) {
        exit(1);
    }

    pthread_create(&self->thread, NULL, message_server_service, (void *) self);

    return self;
}

void message_server_destroy(message_server_t *self) {
    if (self == NULL) { return; }

    if (self->context) {
        lws_context_destroy(self->context);
    }

    free(self);
}

typedef enum input_message_t {
    MESSAGE_DISPLAY = 0,
    MESSAGE_PASTE = 1,
    MESSAGE_COPY = 2,
    MESSAGE_KEY_DOWN = 3,
    MESSAGE_KEY_UP = 4,
    MESSAGE_MOUSE_MOVE = 5,
    MESSAGE_MOUSE_LEFT_DOWN = 6,
    MESSAGE_MOUSE_LEFT_UP = 7,
    MESSAGE_MOUSE_RIGHT_DOWN = 8,
    MESSAGE_MOUSE_RIGHT_UP = 9,
    MESSAGE_MOUSE_MIDDLE_DOWN = 10,
    MESSAGE_MOUSE_MIDDLE_UP = 11,
    MESSAGE_MOUSE_SCROLL = 12,
    MESSAGE_FILE_UPLOAD = 13
}  input_message_t;

void message_server_process(message_server_t *self, client_t *client, char *data) {

    input_message_t message = (*(int *)data);

    switch (message) {

        case MESSAGE_DISPLAY: {

            int *display = (int *)(data + 4);

            if (self->on_change_display != NULL) {
                self->on_change_display(self->user, *display);

                int data_size = LWS_SEND_BUFFER_PRE_PADDING + 8;
                void *data = malloc(data_size);

                memcpy(data + LWS_SEND_BUFFER_PRE_PADDING, &message, 4);
                memcpy(data + LWS_SEND_BUFFER_PRE_PADDING + 4, display, 4);

                lws_callback_all_protocol_vhost_args(lws_get_vhost(client->wsi), lws_get_protocol(client->wsi), LWS_CALLBACK_USER, data, data_size);

                free(data);
            }
        }
        break;

        case MESSAGE_COPY: {

            char *clipboard;

            if (self->on_copy != NULL) {
                self->on_copy(self->user, &clipboard);

                if (clipboard != NULL) {
                    int data_size = LWS_SEND_BUFFER_PRE_PADDING + strlen(clipboard) + 4;
                    void *data = malloc(data_size);

                    memcpy(data + LWS_SEND_BUFFER_PRE_PADDING, &message, 4);
                    memcpy(data + LWS_SEND_BUFFER_PRE_PADDING + 4, clipboard, strlen(clipboard));

                    lws_callback_all_protocol_vhost_args(lws_get_vhost(client->wsi), lws_get_protocol(client->wsi), LWS_CALLBACK_USER, data, data_size);

                    free(data);
                    free(clipboard);
                }
            }
        }
        break;

        case MESSAGE_PASTE: {

            char *contents = (data + 4);

            if (self->on_paste != NULL) {
                self->on_paste(self->user, contents);
            }
        }
        break;

        case MESSAGE_KEY_DOWN: {

            int key = *((int *)(data + 4));

            if (self->on_key_down != NULL) {
                self->on_key_down(self->user, key);
            }
        }
        break;

        case MESSAGE_KEY_UP: {

            int key = *((int *)(data + 4));

            if (self->on_key_up != NULL) {
                self->on_key_up(self->user, key);
            }

        }
        break;

        case MESSAGE_MOUSE_MOVE: {

            double x = *((double *)(data + 8));
            double y = *((double *)(data + 16));

            if (self->on_mouse_move != NULL) {
                self->on_mouse_move(self->user, x, y);
            }
        }
        break;

        case MESSAGE_MOUSE_LEFT_DOWN: {

            if (self->on_mouse_left_down != NULL) {
                self->on_mouse_left_down(self->user);
            }
        }
        break;

        case MESSAGE_MOUSE_LEFT_UP: {

            if (self->on_mouse_left_up != NULL) {
                self->on_mouse_left_up(self->user);
            }
        }
        break;

        case MESSAGE_MOUSE_RIGHT_DOWN: {

            if (self->on_mouse_right_down != NULL) {
                self->on_mouse_right_down(self->user);
            }
        }
        break;

        case MESSAGE_MOUSE_RIGHT_UP: {

            if (self->on_mouse_right_up != NULL) {
                self->on_mouse_right_up(self->user);
            }
        }
        break;

        case MESSAGE_MOUSE_MIDDLE_DOWN: {

            if (self->on_mouse_middle_down != NULL) {
                self->on_mouse_middle_down(self->user);
            }
        }
        break;

        case MESSAGE_MOUSE_MIDDLE_UP: {

            if (self->on_mouse_middle_up != NULL) {
                self->on_mouse_middle_up(self->user);
            }
        }
        break;

        case MESSAGE_MOUSE_SCROLL: {

            int amount = *((int *)(data + 4));

            if (self->on_mouse_scroll != NULL) {
                self->on_mouse_scroll(self->user, amount);
            }
        }
        break;

        case MESSAGE_FILE_UPLOAD: {

            char *filename = (data + 4);
            int contents_size = *((int *)(data + 260));
            char *contents = (data + 264);

            if (self->on_upload_file != NULL) {
                self->on_upload_file(self->user, filename, contents_size, contents);
            }
        }
        break;
    }

}

static int callback_websockets(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *data, size_t len)
{
	struct message_server_t *self = (struct message_server_t *) lws_context_user(lws_get_context(wsi));
	struct client_t *client = (struct client_t *) user;

	switch (reason) {

        case LWS_CALLBACK_SERVER_WRITEABLE: {

            if (!client->authenticated) {
                return 1;
            }

            if (client->send_buffer != NULL && client->send_buffer_size > 0) {
                lws_write(client->wsi, client->send_buffer, client->send_buffer_size, LWS_WRITE_BINARY);

                free(client->send_buffer - LWS_SEND_BUFFER_PRE_PADDING);

                client->send_buffer = NULL;
                client->send_buffer_size = 0;
            }
        }
        break;

		case LWS_CALLBACK_RECEIVE: {

            if (!client->authenticated) {
                return 1;
            }

            // entire message
            if (lws_is_first_fragment(wsi) && lws_is_final_fragment(wsi)) {
                message_server_process(self, client, data);
            // message is being sent in chunks. Write to buffer (in RAM) until completed then callback.
            } else {
                if (lws_is_first_fragment(wsi)){
                    client->receive_buffer = malloc(1024 * 1024);
                    client->receive_buffer_size = 1024 * 1024;
                    client->receive_buffer_position = len;

                    memcpy(&client->receive_buffer[0], data, len);
                } else {
                    if( client->receive_buffer_position + len > client->receive_buffer_size ) {
                        client->receive_buffer_size *= 2;
                        client->receive_buffer = realloc(client->receive_buffer, client->receive_buffer_size);
                    }

                    memcpy(&client->receive_buffer[client->receive_buffer_position], data, len);

                    if (lws_is_final_fragment(wsi)) {
                        message_server_process(self, client, &client->receive_buffer[0]);

                        free(client->receive_buffer);

                        client->receive_buffer = NULL;
                    }

                    client->receive_buffer_position += len;
                }
            }
        }
        break;

        case LWS_CALLBACK_ESTABLISHED: {

            lws_get_peer_simple(wsi, &client->address[0], sizeof(client->address));
            lws_hdr_copy(wsi, &client->password[0], sizeof(client->password), WSI_TOKEN_GET_URI);

            client->authenticated = ((self->password == NULL) || (strcmp(&self->password[0], &client->password[1]) == 0));
            client->wsi = wsi;

            if (client->authenticated) {
                // printf("Client connected: %s\n", client->address);
            } else {
                // printf("Client rejected: %s\n", client->address);

                return 1;
            }
        }
        break;

        case LWS_CALLBACK_CLOSED: {

            if (client->authenticated) {
                // printf("Client disconnected: %s\n", client->address);
            }

            if (client->receive_buffer != NULL) {
                free(client->receive_buffer);
            }

            if (client->send_buffer != NULL) {
                free(client->send_buffer - LWS_SEND_BUFFER_PRE_PADDING);
            }

        }
        break;

        case LWS_CALLBACK_USER: {

            client->send_buffer = malloc(len);

            memcpy(client->send_buffer, data, len);

            client->send_buffer_size = len - LWS_SEND_BUFFER_PRE_PADDING;
            client->send_buffer += LWS_SEND_BUFFER_PRE_PADDING;

            lws_callback_on_writable(client->wsi);
        }
        break;
	}

	return 0;
}
