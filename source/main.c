#include <stdio.h>

#include "app.h"

void exit_usage(char *self_name) {
	printf(
		"Usage: %s [options] <window name>\n\n"

		"Options:\n"
		"  -b bitrate in kilobit/s (default: estimated by output size)\n"
		"  -f target framerate (default: 60)\n"
		"  -p port (default: 8080)\n"
		"  -i enable/disable remote input. E.g. -i 0 (default: 1)\n"
        "  -a authentication password. E.g.: -a pass\n"
        "  -d display number to capture. E.g.: -d 1\n\n",
		self_name
	);
	exit(0);
}

int main(int argc, char* argv[]) {

	int bit_rate = 0,
		fps = 60,
		port = 8080,
		allow_input = 1,
        display_number = 0;

    char *password = NULL;

	// Parse command line options
	for (int i = 1; i < argc; i+=2) {
		switch (argv[i][1]) {
			case 'b': bit_rate = atoi(argv[i+1]) * 1000; break;
			case 'p': port = atoi(argv[i+1]); break;
			case 'f': fps = atoi(argv[i+1]); break;
			case 'i': allow_input = atoi(argv[i+1]); break;
			case 'd': display_number = atoi(argv[i+1]); break;
			case 'a': password = argv[i+1]; break;
            default:  exit_usage(argv[0]);
		}
	}

    XInitThreads();

	app_t *app = app_create(port, display_number, bit_rate, allow_input, password);

	printf(
        "Dimensions: %dx%d\n"
        "Bitrate: %d kb/s\n"
		"Server started on: http://%s:%d\n",
		app->grabber->width, app->grabber->height,
		(int) app->encoder->codec_context->bit_rate / 1000,
		app->stream_server->address, app->stream_server->port
	);

	app_run(app, fps + 1);
	app_destroy(app);

	return 0;
}

