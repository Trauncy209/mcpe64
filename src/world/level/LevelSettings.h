#ifndef NET_MINECRAFT_WORLD_LEVEL__LevelSettings_H__
#define NET_MINECRAFT_WORLD_LEVEL__LevelSettings_H__

//package net.minecraft.world.level;

#include "../../SharedConstants.h"

namespace GameType {
	const int Undefined = -1;
	const int Survival = 0;
	const int Creative = 1;

	const int Default = Creative;
}

class LevelSettings
{
public:
    LevelSettings(long seed, int gameType, int generatorVersion = (int)LGV_ORIGINAL)
    :   seed(seed),
        gameType(validateGameType(gameType)),
        generatorVersion(validateGeneratorVersion(generatorVersion))
    {
    }
	static LevelSettings None() {
		return LevelSettings(-1, -1, -1);
	}

    long getSeed() const {
        return seed;
    }

    int getGameType() const {
        return gameType;
    }

    int getGeneratorVersion() const {
        return generatorVersion;
    }

	//
	// Those two should actually not be here
	// @todo: Move out when we add LevelSettings.cpp :p
	//
	static int validateGameType(int gameType) {
        switch (gameType) {
		case GameType::Creative:
		case GameType::Survival:
            return gameType;
        }
        return GameType::Default;
    }

	static std::string gameTypeToString(int gameType) {
		if (gameType == GameType::Survival) return "Survival";
		if (gameType == GameType::Creative) return "Creative";
		return "Undefined";
	}

	static int validateGeneratorVersion(int generatorVersion) {
		switch (generatorVersion) {
		case LGV_ORIGINAL:
		case LGV_INFINITE:
			return generatorVersion;
		}
		return LGV_ORIGINAL;
	}

	static std::string generatorVersionToString(int generatorVersion) {
		if (generatorVersion == LGV_INFINITE) return "Infinite";
		if (generatorVersion == LGV_ORIGINAL) return "Classic";
		return "Unknown";
	}

private:
    const long seed;
    const int gameType;
    const int generatorVersion;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL__LevelSettings_H__*/
