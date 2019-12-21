#include <stdlib.h>
#include <stdio.h>

#ifdef __linux__
	#include <unistd.h>
	#include <arpa/inet.h>
#endif

#include "stream_server.h"
#include "app.h"

typedef struct frame_t {
    int display_number;
    int cursor_x;
    int cursor_y;
    char data;
} frame_t;

typedef struct client_t {
    struct lws *wsi;

    size_t frame_size;
    frame_t *frame;

    bool authenticated;
	char address[32];
	char password[255];
} client_t;

static int callback_websockets(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

static char *stream_server_get_address() {
	char host_name[80];
	struct hostent *host;
	if (gethostname(host_name, sizeof(host_name)) == -1 || !(host = gethostbyname(host_name))) {
		return "127.0.0.1";
	}
	return inet_ntoa( *(struct in_addr *)(host->h_addr_list[0]) );
}

stream_server_t * stream_server_create(int port, char *password, int buffer_size) {

    stream_server_t *self = (stream_server_t *) malloc(sizeof(stream_server_t));

    memset(self, 0, sizeof(stream_server_t));
    memset(self->protocols, 0, sizeof(self->protocols));

    self->buffer_size = buffer_size;
    self->password = password;
    self->address = stream_server_get_address();
    self->port = port;

    self->protocols[0].name = "http";
    self->protocols[0].callback = callback_http,
    self->protocols[0].per_session_data_size = 0;
    self->protocols[0].rx_buffer_size = 0;

    self->protocols[1].name = "ws";
    self->protocols[1].callback = callback_websockets,
    self->protocols[1].per_session_data_size = sizeof(client_t);
    self->protocols[1].rx_buffer_size = buffer_size;

    self->protocols[2].name = NULL;
    self->protocols[2].callback = NULL,
    self->protocols[2].per_session_data_size = 0;
    self->protocols[2].rx_buffer_size = 0;

    struct lws_context_creation_info info = {0};

	info.port = port;
    info.gid = -1;
    info.uid = -1;
    info.protocols = &self->protocols[0];
    info.user = self;

    self->context = lws_create_context(&info);

    if (!self->context) {
        stream_server_destroy(self);
        return NULL;
    }

    sprintf((char *)&self->cookies, "videoBufferSize=%d", self->buffer_size);

    self->vhost = lws_get_vhost_by_name(self->context, "default");

    return self;
}

void stream_server_destroy(stream_server_t *self) {

    if (self != NULL) {

        if (self->context) {
            lws_context_destroy(self->context);
        }

        free(self);
    }
}

void stream_server_broadcast(stream_server_t *self, void *data, int data_size, int display_number, int cursor_x, int cursor_y)
{
    self->display_number = display_number;
    self->cursor_x = cursor_x;
    self->cursor_y = cursor_y;

    lws_callback_all_protocol_vhost_args(self->vhost, NULL, LWS_CALLBACK_USER, data, data_size);
    lws_callback_on_writable_all_protocol(self->context, &self->protocols[1]);
}

void stream_server_update(stream_server_t *self) {
    lws_service(self->context, -1);
}

void stream_server_idle(stream_server_t *self) {
    lws_service(self->context, 1000);
}

static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    struct stream_server_t *self = (struct stream_server_t *) lws_context_user(lws_get_context(wsi));

    if (reason == LWS_CALLBACK_HTTP) {

        char headers[1024];
        char *headers_pointer = &headers[0];

        if (lws_add_http_header_by_name(wsi, (unsigned char *)"set-cookie:", (unsigned char *)&self->cookies, strlen(self->cookies), (unsigned char *)&headers_pointer, (unsigned char *)headers + sizeof(headers)))
            return 1;

        char *request = (char *)in;

        if (strcmp(request, "/jsmpeg.min.js") == 0)
            return lws_serve_http_file(wsi, "client/jsmpeg.min.js", "text/javascript; charset=utf-8", (char *)&headers, headers_pointer - (char *)&headers);

        if (strcmp(request, "/jsmpeg-vnc.js") == 0)
            return lws_serve_http_file(wsi, "client/jsmpeg-vnc.js", "text/javascript; charset=utf-8", (char *)&headers, headers_pointer - (char *)&headers);

        if (strcmp(request, "/") == 0) {
            return lws_serve_http_file(wsi, "client/index.html", "text/html; charset=utf-8", (char *)&headers, headers_pointer - (char *)&headers);
        }
    }

    return lws_callback_http_dummy(wsi, reason, user, in, len);
}

static int callback_websockets(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	struct stream_server_t *self = (struct stream_server_t *) lws_context_user(lws_get_context(wsi));
    struct client_t *client = (struct client_t *) user;

	switch (reason) {

        case LWS_CALLBACK_USER: {
            client->frame->display_number = self->display_number;
            client->frame->cursor_x = self->cursor_x;
            client->frame->cursor_y = self->cursor_y;
            client->frame_size = len + 12;

            memcpy(&client->frame->data, in, len);
        }
        break;

		case LWS_CALLBACK_SERVER_WRITEABLE: {

            if (!client->authenticated) {
                return 1;
            }

            if (client->frame_size > 0) {
                lws_write(wsi, (unsigned char *) client->frame, client->frame_size, LWS_WRITE_BINARY);
            }

            client->frame_size = 0;
        }
        break;

        case LWS_CALLBACK_ESTABLISHED: {

            lws_get_peer_simple(wsi, &client->address[0], sizeof(client->address));
            lws_hdr_copy(wsi, &client->password[0], sizeof(client->password), WSI_TOKEN_GET_URI);

            client->authenticated = ((self->password == NULL) || (strcmp(&self->password[0], &client->password[1]) == 0));
            client->frame = malloc(self->buffer_size + LWS_PRE);
            client->frame = client->frame + LWS_PRE;

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

            free(client->frame - LWS_PRE);
        }
        break;
	}

	return 0;
}
