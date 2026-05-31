#ifndef NET_MINECRAFT_WORLD_LEVEL__FoliageColor_H__
#define NET_MINECRAFT_WORLD_LEVEL__FoliageColor_H__

//package net.minecraft.world.level;

class FoliageColor
{
public:
	static bool useTint;

	static void setUseTint(bool value) {
       useTint = value;
    }

    static void init(int* p);
    static int get(float temp, float rain);

    static int getEvergreenColor() {
        return 0x2f6f3f;
    }

    static int getBirchColor() {
        return 0x8aaa45;
    }

    static int getDefaultColor() {
        return 0x4f8f32;
    }

private:
    static int* pixels;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL__FoliageColor_H__*/
