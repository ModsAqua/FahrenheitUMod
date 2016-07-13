#include "Settings.h"

Settings Settings::instance;

bool Settings::getIsSMAA() {
	return isSMAA;
}

bool Settings::getIsFXAA() {
	return isFXAA;
}

bool Settings::getIsGaussian() {
	return isGaussian;
}

int Settings::getGaussianType() {
	return gaussianType;
}

int Settings::getGaussianQuality() {
	return gaussianQuality;
}

int Settings::getGaussianSigma() {
	return gaussianSigma;
}

bool Settings::getIsFilmGrain() {
	return isFilmGrain;
}

float Settings::getFilmGrainIntensity() {
	return filmGrainIntensity;
}

int Settings::getFilmGrainExposure() {
	return filmGrainExposure;
}

int Settings::getToggleKey() {
	return toggleKey;
}

int Settings::getScrKey() {
	return scrKey;
}

int Settings::getReloadKey() {
	return reloadKey;
}

std::string Settings::getExternalD3D9Wrapper() {
	return d3D9Wrapper;
}

bool Settings::load() {

	isSMAA = false;	isFXAA = false;	isGaussian = false; isFilmGrain = false;
	d3D9Wrapper = ""; toggleKey = 0; scrKey = 0; reloadKey = 0;

	std::ifstream settingsf("SweetFX_settings.txt");
	if (!settingsf.is_open()) {
		return false;
	}

	unsigned pos = 0;
	std::string line, item;

	// Reading settings
	while (std::getline(settingsf, line))
	{
		if (line.find("#define USE_SMAA_ANTIALIASING 1") != std::string::npos)
			isSMAA = true;

		if (line.find("#define USE_FXAA_ANTIALIASING 1") != std::string::npos)
			isFXAA = true;

		if (line.find("#define USE_GAUSSIAN          1") != std::string::npos)
			isGaussian = true;

		if (line.find("#define USE_FILMGRAIN         1") != std::string::npos)
			isFilmGrain = true;

		if (line.find("GaussEffect") != std::string::npos) {
			item = line.substr(line.find("GaussEffect") +12, 1);
			gaussianType = atoi(item.c_str());
		}

		if (line.find("GaussQuality") != std::string::npos) {
			item = line.substr(line.find("GaussQuality") +13, 1);
			gaussianQuality = atoi(item.c_str());
		}

		if (line.find("GaussSigma") != std::string::npos) {
			item = line.substr(line.find("GaussSigma") +11, 2);
			gaussianSigma = atoi(item.c_str());
		}

		if (line.find("FilmGrainIntensity") != std::string::npos) {
			item = line.substr(line.find("FilmGrainIntensity") +19, 4);
			filmGrainIntensity = (float)atof(item.c_str());
		}

		if (line.find("FilmGrainExposure") != std::string::npos) {
			item = line.substr(line.find("FilmGrainExposure") +18, 3);
			filmGrainExposure = atoi(item.c_str());
		}

		if (line.find("key_toggle_sweetfx") != std::string::npos) {
			item = line.substr(line.find('=') +2, 3);
			toggleKey = atoi(item.c_str());
		}

		if (line.find("key_screenshot") != std::string::npos) {
			item = line.substr(line.find('=') +2, 3);
			scrKey = atoi(item.c_str());
		}

		if (line.find("key_reload_sweetfx") != std::string::npos) {
			item = line.substr(line.find('=') +2, 3);
			reloadKey = atoi(item.c_str());
		}

		if (line.find("external_d3d9_wrapper") != std::string::npos) {
			item = line.substr(line.find('=') +2, 20);
			if (item.compare("none") != 0)
				d3D9Wrapper = item.c_str();
		}
	}

	settingsf.close();

	return true;
}