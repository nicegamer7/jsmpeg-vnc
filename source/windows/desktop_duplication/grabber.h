#ifndef GRABBER_H
#define GRABBER_H

#define COBJMACROS

#include <windows.h>
#include <initguid.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>

typedef struct {
    int width;
    int height;

    ID3D11Device *device;
    ID3D11DeviceContext *context;
    ID3D11Texture2D *staging;
    IDXGIOutputDuplication *duplication;
    D3D11_TEXTURE2D_DESC texture_desc;

    void *buffer;
} grabber_t;

grabber_t *grabber_create(char *display_name);
void grabber_destroy(grabber_t *self);
bool grabber_grab(grabber_t *self);

#endif
