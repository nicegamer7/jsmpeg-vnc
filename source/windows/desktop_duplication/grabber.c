#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "grabber.h"

void error(char *message) {
    printf("Desktop duplication error: %s\n", message);
    exit(100);
}

grabber_t *grabber_create(char *display_name)
{
	grabber_t *self = (grabber_t *)malloc(sizeof(grabber_t));
	memset(self, 0, sizeof(grabber_t));

	D3D_FEATURE_LEVEL feature_level;

	if FAILED(D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, D3D11_CREATE_DEVICE_SINGLETHREADED, 0, 0, D3D11_SDK_VERSION, &self->device, &feature_level, &self->context)) {
        error("D3D11CreateDevice");
	}

    IDXGIDevice *device;
    if FAILED(ID3D11Device_QueryInterface(self->device, &IID_IDXGIDevice, &device)) {
        error("ID3D11Device_QueryInterface");
    }

    IDXGIAdapter *adapater;
    if FAILED(IDXGIObject_GetParent(device, &IID_IDXGIAdapter, &adapater)) {
        error("IDXGIObject_GetParent");
    }

    IDXGIOutput *output;
    if FAILED(IDXGIAdapter_EnumOutputs(adapater, atoi(display_name), &output)) {
        error("IDXGIAdapter_EnumOutputs");
    }

    IDXGIOutput1 *output_again;
    if FAILED(IDXGIOutput_QueryInterface(output, &IID_IDXGIOutput1, &output_again)) {
        error("IDXGIOutput_QueryInterface");
    }

    if FAILED(IDXGIOutput1_DuplicateOutput(output_again, self->device, &self->duplication)) {
        error("IDXGIOutput1_DuplicateOutput");
    }

    DXGI_OUTPUT_DESC output_desc;
    if FAILED(IDXGIOutput_GetDesc(output, &output_desc)) {
        error("IDXGIOutput_GetDesc");
    }

    self->texture_desc.Width = output_desc.DesktopCoordinates.right - output_desc.DesktopCoordinates.left;
    self->texture_desc.Height = output_desc.DesktopCoordinates.bottom - output_desc.DesktopCoordinates.top;
    self->texture_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    self->texture_desc.ArraySize = 1;
    self->texture_desc.MipLevels = 1;
    self->texture_desc.Usage = D3D11_USAGE_STAGING;
    self->texture_desc.SampleDesc.Count = 1;
    self->texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    if FAILED(ID3D11Device_CreateTexture2D(self->device, &self->texture_desc, 0, &self->staging)) {
        error("ID3D11Device_CreateTexture2D");
    }

    self->width = self->texture_desc.Width;
    self->height = self->texture_desc.Height;
    self->buffer = malloc(self->width * self->height * 4);

	return self;
}

void grabber_destroy(grabber_t *self)
{
	if (self == NULL) {
      return;
	}

	IDXGIOutputDuplication_Release(self->duplication);
	ID3D11Texture2D_Release(self->staging);
	ID3D11DeviceContext_Release(self->context);
    ID3D11Device_Release(self->device);
	ID3D11DeviceContext_Release(self->context);

	free(self->buffer);
	free(self);
}

bool grabber_grab(grabber_t *self)
{
    IDXGIResource* desktop_resources = 0;
    DXGI_OUTDUPL_FRAME_INFO frame_info = {};
    HRESULT status = IDXGIOutputDuplication_AcquireNextFrame(self->duplication, 0, &frame_info, &desktop_resources);

    if (status == DXGI_ERROR_WAIT_TIMEOUT) {
        return false;
    }

    if (FAILED(status)) {
        printf("Desktop duplication died\n");
        exit(1);
    }

    ID3D11Texture2D* desktop_texture;
    ID3D11Texture2D_QueryInterface(desktop_resources, &IID_ID3D11Texture2D, &desktop_texture);

    ID3D11DeviceContext_CopyResource(self->context, self->staging, desktop_texture);
    ID3D11DeviceContext_Flush(self->context);

    IDXGIOutputDuplication_ReleaseFrame(self->duplication);

    D3D11_MAPPED_SUBRESOURCE resource;
	ID3D11DeviceContext_Map(self->context, self->staging, 0, D3D11_MAP_READ, 0, &resource);

    memcpy(self->buffer, resource.pData, self->width * self->height * 4);

    ID3D11DeviceContext_Unmap(self->context, self->staging, 0);

    return true;
}

