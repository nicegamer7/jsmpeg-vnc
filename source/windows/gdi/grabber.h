#ifndef GRABBER_H
#define GRABBER_H

#include <windows.h>

typedef struct {
    HWND window;
    HDC windowDC;
    HDC memoryDC;
    HBITMAP bitmap;
    BITMAPINFOHEADER bitmapInfo;
    HGDIOBJ obj;
    int width, height;

    void *buffer;
} grabber_t;

grabber_t *grabber_create(int display_number);
void grabber_destroy(grabber_t *self);
bool grabber_grab(grabber_t *self);

#endif
