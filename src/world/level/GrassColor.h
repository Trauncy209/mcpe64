#ifndef NET_MINECRAFT_WORLD_LEVEL__GrassColor_H__
#define NET_MINECRAFT_WORLD_LEVEL__GrassColor_H__

//package net.minecraft.world.level;

class GrassColor
{
public:
	static bool useTint;

	static void setUseTint(bool value) {
       useTint = value;
    }
/* 
Shredder here, Ive converted the unused commented out code into their correct syntax, though if i did something incorrectly feel free to take reference from the 
commented out code
*/

//     static void init(int[] pixels) {
//         GrassColor::pixels = pixels;
//     }
// 
//     static int get(float temp, float rain) {
//         rain *= temp;
//         int x = (int) ((1 - temp) * 255);
//         int y = (int) ((1 - rain) * 255);
//         return pixels[y << 8 | x];
//     }


     static void init(int* p) {
         pixels = p;
     }
 
    static int get(float temp, float rain);

private:
 //   static int pixels[256*256];
	static int* pixels;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL__GrassColor_H__*/
