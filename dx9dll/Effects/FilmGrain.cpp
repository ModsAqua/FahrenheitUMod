/**
 * Copyright (C) 2012 Jorge Jimenez (jorge@iryoku.com). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are 
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the copyright holders.
 */


#include "FilmGrain.h"
using namespace std;

#define SAFERELEASE(_p) { if(_p) { (_p)->Release(); (_p) = NULL; } }

#pragma region Useful Macros from DXUT (copy-pasted here as we prefer this to be as self-contained as possible)
#if defined(DEBUG) || defined(_DEBUG)
#ifndef V
#define V(x) { hr = (x); }
#endif
#ifndef V_RETURN
#define V_RETURN(x) { hr = (x); }
#endif
#else
#ifndef V
#define V(x) { hr = (x); }
#endif
#ifndef V_RETURN
#define V_RETURN(x) { hr = (x); if( FAILED(hr) ) { return hr; } }
#endif
#endif
#pragma endregion

FilmGrain::FilmGrain(IDirect3DDevice9 *device, int width, int height, float noiseIntensity, float exposure) 
	: Effect(device), width(width), height(height), noiseIntensity(noiseIntensity), exposure(exposure) {

	DWORD flags = D3DXFX_DONOTSAVESTATE | D3DXFX_NOT_CLONEABLE | D3DXSHADER_OPTIMIZATION_LEVEL3;

	// Load effect from file
	ID3DXBuffer* errors;
	HRESULT hr = D3DXCreateEffectFromFile(device, "SweetFX\\Shaders\\GRAIN_DX9.fx", NULL, NULL, flags, NULL, &effect, &errors);
	if ( FAILED(hr) && errors != 0 && errors->GetBufferPointer() != 0 )
	{
		errors->Release();
	}

	V(D3DXCreateTextureFromFile(device, "SweetFX\\Shaders\\GRAIN.dds", &noiseTex));

	pixelSizeHandle      = effect->GetParameterByName(NULL, "pixelSize");
	noiseIntensityHandle = effect->GetParameterByName(NULL, "noiseIntensity");
	exposureHandle       = effect->GetParameterByName(NULL, "exposure");
	tHandle              = effect->GetParameterByName(NULL, "t");
	srcTexHandle         = effect->GetParameterByName(NULL, "srcTex");
	noiseTexHandle       = effect->GetParameterByName(NULL, "noiseTex");

	filmGrainTechnique   = effect->GetTechniqueByName("FilmGrain");
    pixelSize            = D3DXVECTOR2(1.0f / width, 1.0f / height);
}

FilmGrain::~FilmGrain() {
    SAFERELEASE(effect);
	SAFERELEASE(noiseTex);
}

void FilmGrain::go(IDirect3DTexture9 *input, IDirect3DSurface9 *dst, float t) {
	device->SetVertexDeclaration(vertexDeclaration);

	device->SetRenderTarget(0, dst);
	effect->SetFloatArray(pixelSizeHandle, pixelSize, 2);
	effect->SetFloat(noiseIntensityHandle, noiseIntensity);
	effect->SetFloat(exposureHandle, exposure);
	effect->SetFloat(tHandle, t);
	effect->SetTexture(srcTexHandle, input);
	effect->SetTexture(noiseTexHandle, noiseTex);
    effect->SetTechnique(filmGrainTechnique);

    UINT passes;
    effect->Begin(&passes, 0);
    effect->BeginPass(0);
    quad(width, height);
    effect->EndPass();
    effect->End();
}
