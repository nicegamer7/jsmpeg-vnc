#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grabber.h"

grabber_t *grabber_create(int display_number) {

	grabber_t *self = (grabber_t *) malloc(sizeof(grabber_t));
  	memset(self, 0, sizeof(grabber_t));

    char display_name[100] = {0};
    if (display_number > 0) {
        sprintf(display_name, ":%d", display_number);
    }

    self->display = XOpenDisplay(display_name);
    if (self->display == NULL) {
        free(self);

        return NULL;
    }

    self->window = XDefaultRootWindow(self->display);

    XWindowAttributes attri;
  	XGetWindowAttributes(self->display, self->window, &attri);

	self->width = attri.width;
	self->height = attri.height;
	self->image = XShmCreateImage(self->display, XDefaultVisual(self->display, 0), XDefaultDepth(self->display, 0), ZPixmap, NULL, &self->shminfo, self->width, self->height);
	self->shminfo.shmid = shmget(IPC_PRIVATE, self->image->bytes_per_line * self->image->height, IPC_CREAT | 0777);
	self->shminfo.shmaddr = self->image->data = (char *)shmat(self->shminfo.shmid, 0, 0);
	self->shminfo.readOnly = False;

    self->buffer = self->image->data;

	XShmAttach(self->display, &self->shminfo);

	return self;
}

void grabber_destroy(grabber_t *self) {

	if (self != NULL) {

		XShmDetach(self->display, &self->shminfo);
		XDestroyImage(self->image);
		XCloseDisplay(self->display);

		shmdt(self->shminfo.shmaddr);

		free(self);
  	}
}

bool grabber_grab(grabber_t *self) {

  	XShmGetImage(self->display, self->window, self->image, 0, 0, 0x00ffffff);

  	return true;
}
