#include <stdlib.h>
#include <stdio.h>

#ifdef __linux__
	#include <unistd.h>
	#include <arpa/inet.h>
#endif

#include "stream_server.h"
#include "app.h"

typedef struct client_t {
    struct lws *wsi;

    bool authenticated;
	char address[32];
	char password[255];
} client_t;

static int callback_websockets(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

static struct lws_protocols stream_server_protocols[] = {
    {"http", callback_http,           0,                   0},
    {"ws",   callback_websockets,     sizeof(client_t),    APP_FRAME_BUFFER_SIZE},
    {NULL, NULL,                      0,                   0}
};

static char *stream_server_get_address() {
	char host_name[80];
	struct hostent *host;
	if (gethostname(host_name, sizeof(host_name)) == -1 || !(host = gethostbyname(host_name))) {
		return "127.0.0.1";
	}
	return inet_ntoa( *(struct in_addr *)(host->h_addr_list[0]) );
}

stream_server_t * stream_server_create(int port, char *password) {

    stream_server_t *self = (stream_server_t *) malloc(sizeof(stream_server_t));
    memset(self, 0, sizeof(stream_server_t));

    self->password = password;
    self->address = stream_server_get_address();
    self->port = port;
    self->data = malloc(LWS_SEND_BUFFER_PRE_PADDING + sizeof(stream_frame_t));
    self->frame = (struct stream_frame_t *) &self->data[LWS_SEND_BUFFER_PRE_PADDING];

    struct lws_context_creation_info info = {0};

	info.port = port;
    info.gid = -1;
    info.uid = -1;
    info.protocols = stream_server_protocols;
    info.user = self;

    self->context = lws_create_context(&info);

    if (!self->context) {
        stream_server_destroy(self);
        return NULL;
    }

    return self;
}

void stream_server_destroy(stream_server_t *self) {

    if (self != NULL) {

        if (self->context) {
            lws_context_destroy(self->context);
        }

        if (self->data != NULL) {
            free(self->data - LWS_SEND_BUFFER_PRE_PADDING);
        }

        free(self);
    }
}

void stream_server_broadcast(stream_server_t *self, void *data, int data_size, int display_number, int cursor_x, int cursor_y)
{
    self->frame->display_number = display_number;
    self->frame->cursor_x = cursor_x;
    self->frame->cursor_y = cursor_y;

    memcpy(&self->frame->data[0], data, data_size);

    self->frame_size = data_size + 12;

    lws_callback_on_writable_all_protocol(self->context, &stream_server_protocols[1]);
}

void stream_server_update(stream_server_t *self) {
    lws_service(self->context, -1);
}

void stream_server_idle(stream_server_t * self) {
    lws_service(self->context, 1000);
}

static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{

    if (reason == LWS_CALLBACK_HTTP) {
        if (strcmp((char *)in, "/jsmpeg.min.js") == 0) {
            return lws_serve_http_file(wsi, "client/jsmpeg.min.js", "text/javascript; charset=utf-8", NULL, 0);
        }
        if (strcmp((char *)in, "/jsmpeg-vnc.js") == 0 ) {
            return lws_serve_http_file(wsi, "client/jsmpeg-vnc.js", "text/javascript; charset=utf-8", NULL, 0);
        }
        if (strcmp((char *)in, "/") == 0 ) {
            return lws_serve_http_file(wsi, "client/index.html", "text/html; charset=utf-8", NULL, 0);
        }
    }

    return lws_callback_http_dummy(wsi, reason, user, in, len);
}

static int callback_websockets(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	struct stream_server_t *self = (struct stream_server_t *) lws_context_user(lws_get_context(wsi));
    struct client_t *client = (struct client_t *) user;

	switch (reason) {

		case LWS_CALLBACK_SERVER_WRITEABLE: {

            if (!client->authenticated) {
                return 1;
            }

            lws_write(client->wsi, (unsigned char*) self->frame, self->frame_size, LWS_WRITE_BINARY);
        }
        break;

        case LWS_CALLBACK_ESTABLISHED: {

            lws_get_peer_simple(wsi, &client->address[0], sizeof(client->address));
            lws_hdr_copy(wsi, &client->password[0], sizeof(client->password), WSI_TOKEN_GET_URI);

            client->authenticated = ((self->password == NULL) || (strcmp(&self->password[0], &client->password[1]) == 0));
            client->wsi = wsi;

            if (client->authenticated) {
                printf("Client connected: %s (%d active connection(s))\n", client->address, ++self->active_connections);
            } else {
                printf("Client rejected: %s\n", client->address);

                return 1;
            }
        }
        break;

        case LWS_CALLBACK_CLOSED: {

            if (client->authenticated) {
                printf("Client disconnected: %s (%d active connection(s))\n", client->address, --self->active_connections);
            }
        }

	}

	return 0;
}
