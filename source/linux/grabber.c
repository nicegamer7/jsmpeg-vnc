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
        printf("Grabber: Error opening display: %s\n", display_name);
        exit(1);
    }

    if (!XShmQueryExtension(self->display)) {
        printf("Grabber: XShm extension not present\n");
        exit(1);
    }

    self->window = XDefaultRootWindow(self->display);

    XWindowAttributes attri;
    XGetWindowAttributes(self->display, self->window, &attri);

    self->width = attri.width;
    self->height = attri.height;

    self->shminfo.shmid = shmget(IPC_PRIVATE, self->width * self->height * 4, IPC_CREAT | 0777);
    if (self->shminfo.shmid < 0) {
        printf("Grabber: shmget returned NULL\n");
        exit(1);
    }

    self->shminfo.shmaddr = shmat(self->shminfo.shmid, 0, 0);
    if (self->shminfo.shmaddr == NULL) {
        printf("Grabber: shmaddr returned NULL\n");
        exit(1);
    }

    shmctl(self->shminfo.shmid, IPC_RMID, 0);

    self->shminfo.readOnly = False;

    self->image = XShmCreateImage(self->display, XDefaultVisual(self->display, 0), XDefaultDepth(self->display, 0), ZPixmap, NULL, &self->shminfo, self->width, self->height);
    if (self->image == NULL) {
        printf("Grabber: XShmCreateImage returned NULL\n");
        exit(1);
    }

    self->image->data = (unsigned int *) self->shminfo.shmaddr;
    self->buffer = self->image->data;

    XShmAttach(self->display, &self->shminfo);
    XSync(self->display, false);

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
    XShmGetImage(self->display, self->window, self->image, 0, 0, AllPlanes);
    return true;
}
