#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>

#include "Effect.h"

class FXAA : public Effect {
public:
	enum Quality { QualityLow, QualityMedium, QualityHigh, QualityUltra };

    FXAA(IDirect3DDevice9 *device, int width, int height, Quality quality, bool bypassAA);
    virtual ~FXAA();

	void go(IDirect3DTexture9 *frame, IDirect3DSurface9 *dst);

private:
	int width, height;

	ID3DXEffect *effect;
	
	IDirect3DTexture9* buffer1Tex;
	IDirect3DSurface9* buffer1Surf;
	IDirect3DTexture9* tmpRT;
	IDirect3DSurface9* tmpRTSurface;

	D3DXHANDLE ScreenTexHandle;
	
	void finalPass(IDirect3DTexture9 *src, IDirect3DSurface9* dst);
	void lumaPass(IDirect3DSurface9 *dst);
	void fxAAPass(IDirect3DTexture9 *src, IDirect3DSurface9* dst);

	bool bypassAA;
};
