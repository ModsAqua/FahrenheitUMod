#pragma once
#include "Effect.h"

class GAUSS : public Effect {
public:
	enum Type { Blur, Sharpen, Bloom };
	enum Quality { QualityLow, QualityMedium, QualityHigh, QualityUltra, QualityOverKill };

    GAUSS(IDirect3DDevice9 *device, int width, int height, Type type, Quality quality=QualityMedium, int sigma=8);
    virtual ~GAUSS();

	unsigned long upper_power_of_two(unsigned long v);
	void go(IDirect3DTexture9 *input);
	void GAUSS::preprocess(IDirect3DTexture9 *origInput, IDirect3DSurface9 *dst);
	void postprocess(IDirect3DTexture9 *origInput, IDirect3DSurface9 *dst);

private:
	int width, height, dscalewidth, dscaleheigth, numIterations;

	ID3DXEffect *effect;
	
	IDirect3DTexture9* buffer1Tex;
	IDirect3DSurface9* buffer1Surf;
	IDirect3DTexture9* buffer2Tex;
	IDirect3DSurface9* buffer2Surf;

	D3DXHANDLE frameTexHandle;
	D3DXHANDLE frameTexHandleHalf;
	D3DXHANDLE origTexHandle;
};
