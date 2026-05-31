#include "FoliageColor.h"

// Muted, natural leaf color pass. Original 0.6.1 does have foliage tint for
// normal leaves; TruancyCraftPE had fallen back to a fixed color. Keep tree
// foliage green and varied without letting it become blue or neon.
bool FoliageColor::useTint = true;

static int FOLIAGE_FALLBACK_PIXELS[256 * 256];
static int FOLIAGE_TUNED_PIXELS[256 * 256];
static int* foliageSourcePixels = nullptr;
static bool foliageFallbackInitialized = false;
static bool foliageTunedInitialized = false;

static int clampColor(int v) {
	if (v < 0) return 0;
	if (v > 255) return 255;
	return v;
}

static int tuneFoliageColor(int color) {
	int r = (color >> 16) & 0xff;
	int g = (color >> 8) & 0xff;
	int b = color & 0xff;

	if (b > g - 16) {
		b = g - 42;
	}
	if (b > 88) {
		b = 88;
	}
	if (b < 34) {
		b = 34;
	}

	if (g < 96) {
		g = 96;
	}
	if (g > 148) {
		g = 148;
	}
	if (r < 50) {
		r = 50;
	}
	if (r > 116) {
		r = 116;
	}

	return (clampColor(r) << 16) | (clampColor(g) << 8) | clampColor(b);
}

static void initFoliageFallbackPixels() {
	if (foliageFallbackInitialized) {
		return;
	}
	foliageFallbackInitialized = true;
	for (int i = 0; i < 256 * 256; ++i) {
		FOLIAGE_FALLBACK_PIXELS[i] = 0x4d8736;
	}
}

static void initFoliageTunedPixels() {
	if (foliageTunedInitialized) {
		return;
	}
	foliageTunedInitialized = true;
	int* source = foliageSourcePixels;
	if (!source) {
		initFoliageFallbackPixels();
		source = FOLIAGE_FALLBACK_PIXELS;
	}
	for (int i = 0; i < 256 * 256; ++i) {
		FOLIAGE_TUNED_PIXELS[i] = tuneFoliageColor(source[i]);
	}
}

void FoliageColor::init(int* p) {
	foliageSourcePixels = p;
	foliageTunedInitialized = false;
	initFoliageTunedPixels();
	pixels = FOLIAGE_TUNED_PIXELS;
}

int FoliageColor::get(float temp, float rain) {
	if (!pixels) {
		initFoliageTunedPixels();
		pixels = FOLIAGE_TUNED_PIXELS;
	}

	rain *= temp;
	int x = (int) ((1 - temp) * 255);
	int y = (int) ((1 - rain) * 255);
	if (x < 0) x = 0;
	if (x > 255) x = 255;
	if (y < 0) y = 0;
	if (y > 255) y = 255;
	return pixels[y << 8 | x];
}

int* FoliageColor::pixels = nullptr;
