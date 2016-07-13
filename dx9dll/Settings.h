#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <string>

#include <Windows.h>

class Settings 
{
		static Settings instance;

public:
		static Settings& get() {
			return instance;
		}

		bool isSMAA, isFXAA, isGaussian, isFilmGrain;
		int  toggleKey, scrKey, reloadKey, gaussianType, gaussianQuality, gaussianSigma;
		float filmGrainIntensity;
		int filmGrainExposure;
		std::string d3D9Wrapper;

		bool load();
		bool getIsSMAA();
		bool getIsFXAA();
		bool getIsGaussian();
		bool getIsFilmGrain();

		int  getToggleKey();
		int  getScrKey();
		int  getReloadKey();
		int  getGaussianType();
		int  getGaussianQuality();
		int  getGaussianSigma();

		float Settings::getFilmGrainIntensity();
		int   Settings::getFilmGrainExposure();

		std::string getExternalD3D9Wrapper();
};
