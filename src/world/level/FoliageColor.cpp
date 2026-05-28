#include "FoliageColor.h"

bool FoliageColor::useTint = true;

static int FOLIAGE_FALLBACK_PIXELS[256 * 256];

static void initFoliageFallbackPixels() {
	static bool initialized = false;
	if (initialized) {
		return;
	}
	initialized = true;
	for (int i = 0; i < 256 * 256; ++i) {
		FOLIAGE_FALLBACK_PIXELS[i] = 0x48b518;
	}
}

int FoliageColor::get(float temp, float rain) {
	if (!pixels) {
		initFoliageFallbackPixels();
		pixels = FOLIAGE_FALLBACK_PIXELS;
	}

	rain *= temp;
	int x = (int) ((1 - temp) * 255);
	int y = (int) ((1 - rain) * 255);
	return pixels[y << 8 | x];
}

int* FoliageColor::pixels = nullptr;
