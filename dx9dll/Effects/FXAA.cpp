#include "FXAA.h"

#include <string>
#include <sstream>
#include <vector>
using namespace std;

#define SAFERELEASE(_p) { if(_p) { (_p)->Release(); (_p) = NULL; } }

FXAA::FXAA(IDirect3DDevice9 *device, int width, int height, Quality quality, bool bypassAA)
	: Effect(device), width(width), height(height), bypassAA(bypassAA) {

	// Setup the defines for compiling the effect
	vector<D3DXMACRO> defines;
	stringstream s;

	// Setup pixel size macro
	s << "float2(1.0 / " << width << ", 1.0 / " << height << ")";
	string pixelSizeText = s.str();
	s.str("");
	D3DXMACRO pixelSizeMacro = { "PIXEL_SIZE", pixelSizeText.c_str() };
	defines.push_back(pixelSizeMacro);

    // Buffer Width
    s << "((float)" << width << ")";
    string bufferWidthText = s.str();
	s.str("");
    D3DXMACRO bufferWidthMacro = { "BUFFER_WIDTH", bufferWidthText.c_str() };
    defines.push_back(bufferWidthMacro);

    // Buffer Height
    s << "((float)" << height << ")";
    string bufferHeightText = s.str();
	s.str("");
    D3DXMACRO bufferHeightMacro = { "BUFFER_HEIGHT", bufferHeightText.c_str() };
    defines.push_back(bufferHeightMacro);

    // Buffer RCP Width
    s << "(((float)1.0) / " << width << ")";
    string bufferRCPWidthText = s.str();
	s.str("");
    D3DXMACRO bufferRCPWidthMacro = { "BUFFER_RCP_WIDTH", bufferRCPWidthText.c_str() };
    defines.push_back(bufferRCPWidthMacro);

    // Buffer RCP Height
    s << "(((float)1.0) / " << height << ")";
    string bufferRCPHeightText = s.str();
	s.str("");
    D3DXMACRO bufferRCPHeightMacro = { "BUFFER_RCP_HEIGHT", bufferRCPHeightText.c_str() };
    defines.push_back(bufferRCPHeightMacro);

	D3DXMACRO qualityMacros[] = {
		{ "FXAA_QUALITY__PRESET", "10" },
		{ "FXAA_QUALITY__PRESET", "20" },
		{ "FXAA_QUALITY__PRESET", "28" },
		{ "FXAA_QUALITY__PRESET", "39" }
	};
	defines.push_back(qualityMacros[(int)quality]);

	D3DXMACRO null = { NULL, NULL };
	defines.push_back(null);

	DWORD flags = D3DXFX_DONOTSAVESTATE | D3DXFX_NOT_CLONEABLE | D3DXSHADER_OPTIMIZATION_LEVEL3;

	// Load effect from file
	ID3DXBuffer* errors = NULL;
	HRESULT hr = D3DXCreateEffectFromFile(device, "SweetFX\\Shaders\\FXAA_DX9.fx", &defines[0], NULL, flags, NULL, &effect, &errors);
	if ( FAILED(hr) && errors != 0 && errors->GetBufferPointer() != 0 )
	{
		errors->Release();
	}

	// Create buffer
	hr = device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &buffer1Tex, NULL);
	hr = buffer1Tex->GetSurfaceLevel(0, &buffer1Surf);
	hr = device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &tmpRT, NULL);
	hr = tmpRT->GetSurfaceLevel(0, &tmpRTSurface);

	// get handles
	ScreenTexHandle = effect->GetParameterByName(NULL, "frameTex2D");
}

FXAA::~FXAA() {
	SAFERELEASE(effect);
	SAFERELEASE(buffer1Surf);
	SAFERELEASE(buffer1Tex);
	SAFERELEASE(tmpRT);
	SAFERELEASE(tmpRTSurface);
}

void FXAA::go(IDirect3DTexture9 *frame, IDirect3DSurface9 *dst) {
	device->SetVertexDeclaration(vertexDeclaration);

	finalPass(frame, dst);

	if (!bypassAA) {
		lumaPass(buffer1Surf);
		fxAAPass(buffer1Tex, dst);
	}
}

void FXAA::finalPass(IDirect3DTexture9 *src, IDirect3DSurface9* dst) {

	if (!bypassAA)
		device->SetRenderTarget(0, tmpRTSurface);
	else
		device->SetRenderTarget(0, dst);

	// Setup variables
	effect->SetTexture(ScreenTexHandle, src);

	// Do it!
	UINT passes;
	effect->Begin(&passes, 0);
	effect->BeginPass(0);
	quad(width, height);
	effect->EndPass();
	effect->End();
}

void FXAA::lumaPass(IDirect3DSurface9 *dst) {

	device->SetRenderTarget(0, dst);

	// Setup variables
	effect->SetTexture(ScreenTexHandle, tmpRT);

	// Do it!
	UINT passes;
	effect->Begin(&passes, 0);
	effect->BeginPass(1);
	quad(width, height);
	effect->EndPass();
	effect->End();
}

void FXAA::fxAAPass(IDirect3DTexture9 *src, IDirect3DSurface9* dst) {

	device->SetRenderTarget(0, dst);

	// Setup variables
	effect->SetTexture(ScreenTexHandle, src);

	// Do it!
	UINT passes;
	effect->Begin(&passes, 0);
	effect->BeginPass(2);
	quad(width, height);
	effect->EndPass();
	effect->End();
}
