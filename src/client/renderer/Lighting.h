#ifndef NET_MINECRAFT_CLIENT_RENDERER__Lighting_H__
#define NET_MINECRAFT_CLIENT_RENDERER__Lighting_H__

class Minecraft;
class Lighting
{
public:
	static void turnOff();
	static void turnOn(Minecraft* minecraft);

};
#endif /*NET_MINECRAFT_CLIENT_RENDERER__Lighting_H__*/