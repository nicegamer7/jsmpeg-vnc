#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <X11/Xlib.h>

#include "os.h"

bool os_is_display(int display_number) {

    if (display_number < 0) {
        return false;
    }

    char display_name[100] = {0};
    if (display_number > 0) {
        sprintf(display_name, ":%d", display_number);
    }

    Display *display = XOpenDisplay(display_name);
    bool res = display != NULL;
    if (display != NULL) {
        XCloseDisplay(display);
    }

    return res;
}

void os_set_clipboard(char *contents, int display_number)
{
    if (system("command -v xclip") == 0) {
        FILE *file = fopen(".clipboard", "w+");
        fwrite(contents, sizeof(char), strlen(contents), file);
        fclose(file);

        char command[255] = {0};
        sprintf(command, "cat .clipboard | DISPLAY=:%d xclip -i -selection clipboard", display_number);
        system(command);
    } else {
        printf("ERROR: Unable to set clipboard. Install xclip.\n");
    }
}

char *os_get_clipboard(int display_number)
{
    if (system("command -v xclip") == 0) {

        char command[255] = {0};
        sprintf(command, "DISPLAY=:%d xclip -o -sel clip > .clipboard", display_number);
        system(command);

        FILE *file = fopen(".clipboard", "rb");
        fseek(file, 0, SEEK_END);
        int size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *buffer = malloc(size + 1);
        fread(buffer, 1, size, file);
        fclose(file);

        buffer[size] = 0;

        return buffer;
    } else {
        printf("ERROR: Unable to get clipboard. Install xclip.\n");

        return NULL;
    }
}

double os_get_time(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000000.0 + ts.tv_nsec) / 1000000;
}

void os_sleep(int milliseconds)
{
   struct timespec req, rem;

   if (milliseconds > 999)
   {
        req.tv_sec = (int)(milliseconds / 1000);
        req.tv_nsec = (milliseconds - ((long)req.tv_sec * 1000)) * 1000000;
   }
   else
   {
        req.tv_sec = 0;
        req.tv_nsec = milliseconds * 1000000;
   }

   nanosleep(&req, &rem);
}

void os_save_upload(char *contents, int size, char *filename)
{

    char destination[256] = {0};
    sprintf(destination, "%s/Downloads/%s", getenv("HOME"), filename);

    FILE *file = fopen(destination, "w+");
    fwrite(contents, sizeof(char), size, file);
    fclose(file);

    printf("\nFile uploaded: %s\n", destination);
}
