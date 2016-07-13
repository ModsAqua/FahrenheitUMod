#pragma once
#include "uMod_IDirect3DDevice9.h"

class ID3D9SwapChain : public IDirect3DSwapChain9
{
	public:
		ID3D9SwapChain(IDirect3DSwapChain9* pSwapChain, IDirect3DDevice9* pDevice, uMod_IDirect3DDevice9* pDeviceProxy);
		virtual ~ID3D9SwapChain(void);

		//The original IDirect3DSwapChain9 definitions
		HRESULT WINAPI QueryInterface(REFIID riid, void** ppvObj);
		ULONG WINAPI AddRef(void);
		ULONG WINAPI Release(void);
		HRESULT WINAPI Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
		HRESULT WINAPI GetFrontBufferData(IDirect3DSurface9* pDestSurface);
		HRESULT WINAPI GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer);
		HRESULT WINAPI GetRasterStatus(D3DRASTER_STATUS* pRasterStatus);
		HRESULT WINAPI GetDisplayMode(D3DDISPLAYMODE* pMode);
		HRESULT WINAPI GetDevice(IDirect3DDevice9** ppDevice);
		HRESULT WINAPI GetPresentParameters(D3DPRESENT_PARAMETERS* pPresentationParameters);

		static ID3D9SwapChain *lastDevice;

private:
		IDirect3DSwapChain9 *swapchain;
		uMod_IDirect3DDevice9 *device;
};

