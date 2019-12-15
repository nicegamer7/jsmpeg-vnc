#ifndef GRABBER_H
#define GRABBER_H

#include <Windows.h>

typedef struct {

	HWND window;
	HDC windowDC;
	HDC memoryDC;
	HBITMAP bitmap;
	BITMAPINFOHEADER bitmapInfo;

	int width, height;

	void *buffer;
} grabber_t;

grabber_t *grabber_create(char *display_name);
void grabber_destroy(grabber_t *self);
bool grabber_grab(grabber_t *self);

#endif
