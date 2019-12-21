#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "grabber.h"

grabber_t *grabber_create(int display_number)
{
	grabber_t *self = (grabber_t *)malloc(sizeof(grabber_t));
	memset(self, 0, sizeof(grabber_t));

	self->window = GetDesktopWindow();

	RECT rect;
	GetWindowRect(self->window, &rect);

	self->width = rect.right - rect.left;
	self->height = rect.bottom - rect.top;
	self->windowDC = GetDC(self->window);
	self->memoryDC = CreateCompatibleDC(self->windowDC);
	self->bitmap = CreateCompatibleBitmap(self->windowDC, self->width, self->height);
    self->obj = SelectObject(self->memoryDC, self->bitmap);
	self->bitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
	self->bitmapInfo.biPlanes = 1;
	self->bitmapInfo.biBitCount = 32;
	self->bitmapInfo.biWidth = self->width;
	self->bitmapInfo.biHeight = -self->height;
	self->bitmapInfo.biCompression = BI_RGB;
	self->bitmapInfo.biSizeImage = 0;
	self->buffer = malloc(self->width * self->height * 4);

	return self;
}

void grabber_destroy(grabber_t *self)
{
	if (self == NULL) {
      return;
	}

    SelectObject(self->memoryDC, self->obj);
    DeleteDC(self->windowDC);

	free(self->buffer);
	free(self);
}

bool grabber_grab(grabber_t *self)
{
    BitBlt(self->memoryDC, 0, 0, self->width, self->height, self->windowDC, 0, 0, SRCCOPY);
    GetDIBits(self->memoryDC, self->bitmap, 0, self->height, self->buffer, (BITMAPINFO*)&(self->bitmapInfo), DIB_RGB_COLORS);

    return true;
}

