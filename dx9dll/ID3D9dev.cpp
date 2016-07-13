#include "uMod_IDirect3DDevice9.h"
#include "ID3D9swapchain.h"
#include "Settings.h"

#define SAFEDELETE(_p) { if(_p) { delete (_p); (_p) = NULL; } }
#define SAFERELEASE(_p) { if(_p) { (_p)->Release(); (_p) = NULL; } }

void uMod_IDirect3DDevice9::ConstructorSFX(D3DPRESENT_PARAMETERS *pPresentParam) {
	Settings::get().load();

	SFXenabled = UseEndSceneSFX = UsePresentSFX = UseSwpChPresentSFX = false;
	fxaaSFX = NULL;
	gaussSFX = NULL;
	filmGrainSFX = NULL;
	rgbaBuffer1TexSFX = NULL;
	rgbaBuffer1SurfSFX = NULL;
	prevStateBlockSFX = NULL;
	blurSurfSFX = NULL;
	blurTexSFX = NULL;
	CounterFreqSFX = 0.0;

	presentParamsSFX = *pPresentParam;	// Store display information

	InitSFX();
}

void uMod_IDirect3DDevice9::InitSFX() {
	// Reading present params values
	int bw = presentParamsSFX.BackBufferWidth;
	int bh = presentParamsSFX.BackBufferHeight;	

	if (bw > 2 && bh > 2) {
		fxaaSFX = new FXAA(m_pIDirect3DDevice9, bw, bh, FXAA::QualityUltra, false); // Quick hack : use SweetFX shaders without any AA applied

																					// Gaussian Blur (Bloom/Blur...)
		if (Settings::get().getIsGaussian()) {
			gaussSFX = new GAUSS(m_pIDirect3DDevice9, bw, bh, (GAUSS::Type)Settings::get().getGaussianType(), (GAUSS::Quality)Settings::get().getGaussianQuality(), Settings::get().getGaussianSigma());

			m_pIDirect3DDevice9->CreateTexture(bw, bh, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &blurTexSFX, NULL);
			blurTexSFX->GetSurfaceLevel(0, &blurSurfSFX);
		}

		// Film grain...
		if (Settings::get().getIsFilmGrain()) {
			float exposure = 5.0f * float(Settings::get().getFilmGrainExposure()) / 100;
			StartCounterSFX();
			filmGrainSFX = new FilmGrain(m_pIDirect3DDevice9, bw, bh, Settings::get().getFilmGrainIntensity(), exposure);
		}

		// Preparing our tex & surfaces once for all
		m_pIDirect3DDevice9->CreateTexture(bw, bh, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &rgbaBuffer1TexSFX, NULL);
		rgbaBuffer1TexSFX->GetSurfaceLevel(0, &rgbaBuffer1SurfSFX);
		m_pIDirect3DDevice9->CreateStateBlock(D3DSBT_ALL, &prevStateBlockSFX);

		SFXenabled = true;
	}
}

void uMod_IDirect3DDevice9::AddSFXInject(IDirect3DSurface9 *surface) {
	// Main code-block : activates AA and shaders... (performance is crucial)
	if (SFXenabled) {
		// Maps the captured frame to an exploitable surface/texture
		m_pIDirect3DDevice9->StretchRect(surface, NULL, rgbaBuffer1SurfSFX, NULL, D3DTEXF_POINT);

		prevStateBlockSFX->Capture();

		// Disable any renderstates that could interfere with AA
		m_pIDirect3DDevice9->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		m_pIDirect3DDevice9->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		m_pIDirect3DDevice9->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		m_pIDirect3DDevice9->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
		m_pIDirect3DDevice9->SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
		m_pIDirect3DDevice9->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
		m_pIDirect3DDevice9->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

		if (filmGrainSFX) {
			filmGrainSFX->go(rgbaBuffer1TexSFX, surface, 2.5f * float(GetCounterSFX()));
			m_pIDirect3DDevice9->StretchRect(surface, NULL, rgbaBuffer1SurfSFX, NULL, D3DTEXF_POINT);
		}

		fxaaSFX->go(rgbaBuffer1TexSFX, surface);

		if (gaussSFX) {
			m_pIDirect3DDevice9->StretchRect(surface, NULL, rgbaBuffer1SurfSFX, NULL, D3DTEXF_POINT);

			// Saves the vanilla picture into a new tex before processing
			m_pIDirect3DDevice9->StretchRect(surface, NULL, blurSurfSFX, NULL, D3DTEXF_POINT);

			if ((GAUSS::Type)Settings::get().getGaussianType() == GAUSS::Bloom) {
				gaussSFX->preprocess(rgbaBuffer1TexSFX, surface);
				m_pIDirect3DDevice9->StretchRect(surface, NULL, rgbaBuffer1SurfSFX, NULL, D3DTEXF_POINT);
			}
			// Loops as many times as needed according to the iterations settings
			gaussSFX->go(rgbaBuffer1TexSFX);

			// Post-blur pass
			gaussSFX->postprocess(blurTexSFX, surface);
			m_pIDirect3DDevice9->StretchRect(surface, NULL, rgbaBuffer1SurfSFX, NULL, D3DTEXF_POINT);
		}

		// Restore State...
		prevStateBlockSFX->Apply();

		surface->Release();
	}

	// Activate/Disable AAEnabled
	if (GetAsyncKeyState(Settings::get().getToggleKey())) {
		SFXenabled = !SFXenabled;
	}
	// Reloads shaders
	else if (GetAsyncKeyState(Settings::get().getReloadKey())) {
		releaseSFXResources();
		InitSFX();
	}
}

HRESULT uMod_IDirect3DDevice9::EndScene(void) {
	UseEndSceneSFX = true;
	if (UseEndSceneSFX && !UsePresentSFX && !UseSwpChPresentSFX) {
		IDirect3DSurface9 *renderTarget = NULL;
		m_pIDirect3DDevice9->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &renderTarget);
		AddSFXInject(renderTarget);
		SAFERELEASE(renderTarget);
	}

	return(m_pIDirect3DDevice9->EndScene());
}

HRESULT uMod_IDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {
	// Critical release
	releaseSFXResources();

	HRESULT hr = (m_pIDirect3DDevice9->Reset(pPresentationParameters));
	if (hr == D3D_OK)
	{
		presentParamsSFX = *pPresentationParameters;
		this->InitSFX();
	}

	return hr;
}

HRESULT uMod_IDirect3DDevice9::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
	UsePresentSFX = true;
	if ((UsePresentSFX && !UseEndSceneSFX) || (UseEndSceneSFX && UsePresentSFX)) {
		// When both EndScene() and Present() are used by the game, 
		// we still favour applying AA in Present() (otherwise its very slow in EndScene() -> ie. RE5, nullDC...)
		IDirect3DSurface9 *renderTarget = NULL;
		m_pIDirect3DDevice9->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &renderTarget);
		AddSFXInject(renderTarget);
		SAFERELEASE(renderTarget);
	}

	return (m_pIDirect3DDevice9->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion));
}


HRESULT uMod_IDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) {
	if (pPresentationParameters->BackBufferWidth != presentParamsSFX.BackBufferWidth &&
		pPresentationParameters->BackBufferHeight != presentParamsSFX.BackBufferHeight) {
		memcpy(&presentParamsSFX, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
		releaseSFXResources();
		InitSFX();
	}
	ID3D9SwapChain *swpChain = NULL;
	HRESULT hr = m_pIDirect3DDevice9->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);

	if (hr == D3D_OK)
		// Hooking the swapchain...
		swpChain = new ID3D9SwapChain(*pSwapChain, m_pIDirect3DDevice9, this);

	//Replace returned swapchain with our fake swapchain
	*pSwapChain = swpChain;

	return hr;
}

void uMod_IDirect3DDevice9::releaseSFXResources() {
	SFXenabled = false;
	SAFERELEASE(rgbaBuffer1SurfSFX);
	SAFERELEASE(rgbaBuffer1TexSFX);
	SAFERELEASE(prevStateBlockSFX);
	SAFEDELETE(fxaaSFX);
	SAFEDELETE(filmGrainSFX);
}

// For filmgrain...
void uMod_IDirect3DDevice9::StartCounterSFX() {
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	CounterFreqSFX = double(li.QuadPart);        // Seconds

	QueryPerformanceCounter(&li);
	CounterStartSFX = li.QuadPart;
}

double uMod_IDirect3DDevice9::GetCounterSFX() {
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - CounterStartSFX) / CounterFreqSFX;
}
