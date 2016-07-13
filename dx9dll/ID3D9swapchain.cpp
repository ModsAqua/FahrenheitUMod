#include "ID3D9swapchain.h"
#include "uMod_IDirect3DDevice9.h"

#define SAFERELEASE(_p) { if(_p) { (_p)->Release(); (_p) = NULL; } }

ID3D9SwapChain *ID3D9SwapChain::lastDevice = NULL;

ID3D9SwapChain::ID3D9SwapChain(IDirect3DSwapChain9* pSwapChain, IDirect3DDevice9* pDevice, uMod_IDirect3DDevice9* pDeviceProxy)
{
	swapchain = pSwapChain;
	device = pDeviceProxy;
	lastDevice = this;
}

ID3D9SwapChain::~ID3D9SwapChain(void){
	lastDevice = NULL;
	SAFERELEASE(device);
	SAFERELEASE(swapchain);
}

HRESULT ID3D9SwapChain::QueryInterface(REFIID riid, void** ppvObj) {
	return(swapchain->QueryInterface(riid, ppvObj));
}

ULONG ID3D9SwapChain::AddRef() {
	return(swapchain->AddRef());
}

ULONG ID3D9SwapChain::Release(THIS) {
	return swapchain->Release();
}

HRESULT ID3D9SwapChain::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion,DWORD dwFlags)
{
	device->UseSwpChPresentSFX = true;
	IDirect3DSurface9 *renderTarget = NULL;
	if (SUCCEEDED(swapchain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &renderTarget)))
	{
		device->AddSFXInject(renderTarget);
	}
	SAFERELEASE(renderTarget);

	return(swapchain->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags));
}

HRESULT ID3D9SwapChain::GetFrontBufferData(IDirect3DSurface9* pDestSurface) {
	return(swapchain->GetFrontBufferData(pDestSurface));
}

HRESULT ID3D9SwapChain::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
	return(swapchain->GetBackBuffer(iBackBuffer, Type, ppBackBuffer));
}

HRESULT ID3D9SwapChain::GetRasterStatus(D3DRASTER_STATUS* pRasterStatus) {
	return(swapchain->GetRasterStatus(pRasterStatus));
}

HRESULT ID3D9SwapChain::GetDisplayMode(D3DDISPLAYMODE* pMode) {
	return(swapchain->GetDisplayMode(pMode));
}

HRESULT ID3D9SwapChain::GetDevice(IDirect3DDevice9** ppDevice) {
	return(swapchain->GetDevice(ppDevice));
}

HRESULT ID3D9SwapChain::GetPresentParameters(D3DPRESENT_PARAMETERS* pPresentationParameters) {
	return(swapchain->GetPresentParameters(pPresentationParameters));
}
