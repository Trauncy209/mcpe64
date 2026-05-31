#include "GrassColor.h"

// Keep biome tinting, but remap the source grass color table away from
// cyan/blue water-like tones into muted natural greens. Some legacy PE color
// table samples are very blue in cool/wet biomes; grass tops should stay green.
bool GrassColor::useTint = true;

static int GRASS_FALLBACK_PIXELS[256 * 256];
static int GRASS_TUNED_PIXELS[256 * 256];
static int* grassSourcePixels = nullptr;
static bool grassFallbackInitialized = false;
static bool grassTunedInitialized = false;

static int clampColor(int v) {
	if (v < 0) return 0;
	if (v > 255) return 255;
	return v;
}

static int tuneGrassColor(int color) {
	int r = (color >> 16) & 0xff;
	int g = (color >> 8) & 0xff;
	int b = color & 0xff;

	// Remove the blue/cyan cast first. Grass can be cool, but blue should never
	// dominate the green channel like water.
	if (b > g - 10) {
		b = g - 34;
	}
	if (b > 96) {
		b = 96;
	}
	if (b < 36) {
		b = 36;
	}

	// Keep the top grass green, but avoid neon/saturated bright-green output.
	if (g < 102) {
		g = 102;
	}
	if (g > 156) {
		g = 156;
	}
	if (r < 54) {
		r = 54;
	}
	if (r > 126) {
		r = 126;
	}

	// If the source sample was extremely cold/blue, pull it toward an olive
	// forest-green instead of preserving the bad hue.
	if (b > r + 18) {
		r = (r * 2 + 86) / 3;
		g = (g * 2 + 132) / 3;
		b = (b + 56) / 2;
	}

	return (clampColor(r) << 16) | (clampColor(g) << 8) | clampColor(b);
}

static void initGrassFallbackPixels() {
	if (grassFallbackInitialized) {
		return;
	}
	grassFallbackInitialized = true;
	for (int i = 0; i < 256 * 256; ++i) {
		GRASS_FALLBACK_PIXELS[i] = 0x4f8f3a;
	}
}

static void initGrassTunedPixels() {
	if (grassTunedInitialized) {
		return;
	}
	grassTunedInitialized = true;
	int* source = grassSourcePixels;
	if (!source) {
		initGrassFallbackPixels();
		source = GRASS_FALLBACK_PIXELS;
	}
	for (int i = 0; i < 256 * 256; ++i) {
		GRASS_TUNED_PIXELS[i] = tuneGrassColor(source[i]);
	}
}

void GrassColor::init(int* p) {
	grassSourcePixels = p;
	grassTunedInitialized = false;
	initGrassTunedPixels();
	pixels = GRASS_TUNED_PIXELS;
}

int GrassColor::get(float temp, float rain) {
	if (!pixels) {
		initGrassTunedPixels();
		pixels = GRASS_TUNED_PIXELS;
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

int* GrassColor::pixels = nullptr;
