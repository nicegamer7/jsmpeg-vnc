#include <stdio.h>

#include "app.h"

void exit_usage(char *self_name) {
	printf(
        "\n"
		"Usage: %s [options]\n\n"

		"Options:\n"
		"  -bitrate               bitrate in kilobit/s. (default: estimated by output size)\n"
		"  -fps                   target framerate. (default: 60)\n"
		"  -port                  streaming port. (default: 8080)\n"
		"  -allow_input           enable/disable remote input. (default: 1)\n"
        "  -password password     required for socket authentication. (default: no password)\n"
        "  -display_number        display number to capture. (default: 0)\n"
        "  -gop                   group of pictures. (default 18)\n"
        "  -video_buffer_size     video buffer size. (default 768x768)\n",
		self_name
	);
	exit(0);
}

int main(int argc, char* argv[]) {

	int bit_rate = 0,
		fps = 60,
		port = 8080,
		allow_input = 1,
        display_number = 0,
        video_buffer_size = 768 * 768,
        gop = 18;

    char *password = NULL;

	for (int i = 1; i < argc; i += 2) {

        if (argv[i][0] != '-') {
			exit_usage(argv[0]);
		}

        char *parameter = &argv[i][1];
        char *value = argv[i + 1];

        if (strcmp(parameter, "gop") == 0) {
            gop = atoi(value);
        }
        else
		if (strcmp(parameter, "bitrate") == 0) {
            bit_rate = atoi(value) * 1000;
		}
		else
		if (strcmp(parameter, "fps") == 0) {
            fps = atoi(value);
		}
		else
		if (strcmp(parameter, "password") == 0) {
            password = value;
		}
		else
		if (strcmp(parameter, "display_number") == 0) {
            display_number = atoi(value);
		}
		else
		if (strcmp(parameter, "allow_input") == 0) {
            allow_input = atoi(value);
		}
		else
        if (strcmp(parameter, "port") == 0) {
            port = atoi(value);
		}
		else
        if (strcmp(parameter, "video_buffer_size") == 0) {
            int left, right;
            sscanf(value, "%dx%d", &left, &right);
            video_buffer_size = left * right;
		} else {
            exit_usage(argv[0]);
		}
	}

#ifdef __linux__
    XInitThreads();
#endif

	app_t *app = app_create(port, display_number, bit_rate, allow_input, password, video_buffer_size, gop);

	printf(
        "Dimensions: %dx%d\n"
        "Bitrate: %d kb/s\n"
		"Server started on: http://%s:%d\n",
		app->grabber->width, app->grabber->height,
		app->encoder->codec_context->bit_rate / 1000,
		app->stream_server->address, app->stream_server->port
	);

	app_run(app, fps + 1);
	app_destroy(app);

	return 0;
}

