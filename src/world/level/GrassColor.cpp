#include "GrassColor.h"

// TODO: Probably move all the stuff from the header into here so it's a bit cleaner
bool GrassColor::useTint = true;

static int GRASS_FALLBACK_PIXELS[256 * 256];

static void initGrassFallbackPixels() {
	static bool initialized = false;
	if (initialized) {
		return;
	}
	initialized = true;
	for (int i = 0; i < 256 * 256; ++i) {
		GRASS_FALLBACK_PIXELS[i] = 0x339933;
	}
}

int GrassColor::get(float temp, float rain) {
	if (!pixels) {
		initGrassFallbackPixels();
		pixels = GRASS_FALLBACK_PIXELS;
	}

	rain *= temp;
	int x = (int) ((1 - temp) * 255);
	int y = (int) ((1 - rain) * 255);
	return pixels[y << 8 | x];
}

int* GrassColor::pixels = nullptr; 