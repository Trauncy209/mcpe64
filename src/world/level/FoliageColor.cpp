#include "FoliageColor.h"

// TODO: Probably move all the stuff from the header into here so it's a bit cleaner
bool FoliageColor::useTint = true;

int FoliageColor::get(float temp, float rain) {
	rain *= temp;
	int x = (int) ((1 - temp) * 255);
	int y = (int) ((1 - rain) * 255);
	return pixels[y << 8 | x];
}

int* FoliageColor::pixels = nullptr; 