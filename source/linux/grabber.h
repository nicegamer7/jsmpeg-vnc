#ifndef GRABBER_H
#define GRABBER_H

#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

typedef struct {
    int width;
    int height;
    Window window;
    Display *display;
    XImage *image;
    XShmSegmentInfo shminfo;
    char *buffer;
} grabber_t;

grabber_t *grabber_create(int display_number);
void grabber_destroy(grabber_t *self);
bool grabber_grab(grabber_t *self);

#endif
