#include "GAUSS.h"
#include <string>
#include <sstream>
#include <vector>
using namespace std;

GAUSS::GAUSS(IDirect3DDevice9 *device, int width, int height, Type type, Quality quality, int sigma)
	: Effect(device), width(width), height(height), numIterations(0) {

	if (type == GAUSS::Bloom)
		sigma *= 4;

	if (type == GAUSS::Sharpen)
	{
		dscalewidth  = width;
		dscaleheigth = height;
	}
	else {
		dscalewidth  = upper_power_of_two(width  / ((sigma == 0) ? 1 : sigma));
		dscaleheigth = upper_power_of_two(height / ((sigma == 0) ? 1 : sigma));
	}

	switch((int)quality) {
        case 0:
			// Low quality
			numIterations = 1;
			break;
        case 1:
			// Medium quality
			numIterations = 3;
			break;
        case 2:
			// High quality
			numIterations = 9;
			break;
        case 3:
			// High quality
			numIterations = 12;
			break;
        case 4:
			// High quality
			numIterations = 96;
			break;
		default:
			// Low quality
			numIterations = 1;
			break;
	}

	// Setup the defines for compiling the effect
    vector<D3DXMACRO> defines;
    stringstream s;

    // Setup pixel size macro
    s << "float2(1.0 / " << dscalewidth << ", 1.0 / " << dscaleheigth << ")";
    string pixelSizeText = s.str();
    D3DXMACRO pixelSizeMacro = { "PIXEL_SIZE", pixelSizeText.c_str() };
    defines.push_back(pixelSizeMacro);

    //D3DXMACRO LvlMacro = { "SMAA_HLSL_3", "1" };
    //defines.push_back(LvlMacro);

    D3DXMACRO null = { NULL, NULL };
    defines.push_back(null);

	DWORD flags = D3DXFX_DONOTSAVESTATE | D3DXFX_NOT_CLONEABLE | D3DXSHADER_OPTIMIZATION_LEVEL3;

	// Load effect from file
	ID3DXBuffer* errors;
	HRESULT hr = D3DXCreateEffectFromFile(device, "SweetFX\\Shaders\\GAUSS_DX9.fx", &defines[0], NULL, flags, NULL, &effect, &errors);
	if ( FAILED(hr) && errors != 0 && errors->GetBufferPointer() != 0 )
	{
		errors->Release();
	}

	// Create buffers
	device->CreateTexture(dscalewidth, dscaleheigth, 1, D3DUSAGE_RENDERTARGET|D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &buffer1Tex, NULL);
    buffer1Tex->GetSurfaceLevel(0, &buffer1Surf);
	device->CreateTexture(dscalewidth, dscaleheigth, 1, D3DUSAGE_RENDERTARGET|D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &buffer2Tex, NULL);
    buffer2Tex->GetSurfaceLevel(0, &buffer2Surf);
	
	// get handles
	frameTexHandle = effect->GetParameterByName(NULL, "frameTex2D");
	origTexHandle  = effect->GetParameterByName(NULL, "origframeTex2D");
}

GAUSS::~GAUSS() {
	SAFERELEASE(effect);
	SAFERELEASE(buffer1Surf);
	SAFERELEASE(buffer1Tex);
	SAFERELEASE(buffer2Surf);
	SAFERELEASE(buffer2Tex);
}

// http://graphics.stanford.edu/~seander/bithacks.html
unsigned long GAUSS::upper_power_of_two(unsigned long v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

void GAUSS::preprocess(IDirect3DTexture9 *origInput, IDirect3DSurface9 *dst) {
	device->SetVertexDeclaration(vertexDeclaration);

    UINT passes;

    // Combine
	device->SetRenderTarget(0, dst);

    effect->SetTexture(frameTexHandle, origInput);
	effect->Begin(&passes, 0);
	effect->BeginPass(0);
	quad(width, height);
	effect->EndPass();
	effect->End();
}

void GAUSS::go(IDirect3DTexture9 *input) {
	device->SetVertexDeclaration(vertexDeclaration);

    UINT passes;

	for(int i = 0; i < numIterations; ++i) {
		// Horizontal blur
		device->SetRenderTarget(0, buffer1Surf);

		if (i == 0)
			// We process the original frame
			effect->SetTexture(frameTexHandle, input);
		else
			// We process our already gaussian'd picture (a 2nd, 3rd time...)
			effect->SetTexture(frameTexHandle, buffer2Tex);

		effect->Begin(&passes, 0);
		effect->BeginPass(1);
		quad(dscalewidth, dscaleheigth);
		effect->EndPass();
		effect->End();

		// Vertical blur
		device->SetRenderTarget(0, buffer2Surf);

		effect->SetTexture(frameTexHandle, buffer1Tex);
		effect->Begin(&passes, 0);
		effect->BeginPass(2);
		quad(dscalewidth, dscaleheigth);
		effect->EndPass();
		effect->End();
	}
}

void GAUSS::postprocess(IDirect3DTexture9 *origInput, IDirect3DSurface9 *dst) {
	
    UINT passes;

    // Combine
	device->SetRenderTarget(0, dst);

    effect->SetTexture(origTexHandle, origInput);
    effect->SetTexture(frameTexHandle, buffer2Tex);
	effect->Begin(&passes, 0);
	effect->BeginPass(3);
	quad(width, height);
	effect->EndPass();
	effect->End();
}
