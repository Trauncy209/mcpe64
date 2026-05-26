# TruancyCraftPE

TruancyCraftPE is a community restoration and expansion of **Minecraft: Pocket Edition 0.6.1 alpha**.

This project is about keeping the soul of old MCPE while pushing it forward on modern devices.

**Our philosophy:** this should be a **community-made Minecraft, not corporate slop**.

We want to preserve what made early Pocket Edition special, fix what was broken, and steadily build a better version of it in public.

[![Download Latest Release](https://img.shields.io/badge/Download-Latest%20Release-2ea44f?style=for-the-badge)](https://github.com/Trauncy209/TruancyCraftPE/releases/latest)
[![GitHub release](https://img.shields.io/github/v/release/Trauncy209/TruancyCraftPE?display_name=tag&style=for-the-badge)](https://github.com/Trauncy209/TruancyCraftPE/releases/latest)
[![GitHub stars](https://img.shields.io/github/stars/Trauncy209/TruancyCraftPE?style=for-the-badge)](https://github.com/Trauncy209/TruancyCraftPE/stargazers)
[![GitHub watchers](https://img.shields.io/github/watchers/Trauncy209/TruancyCraftPE?style=for-the-badge)](https://github.com/Trauncy209/TruancyCraftPE/watchers)

**Download here:** https://github.com/Trauncy209/TruancyCraftPE/releases/latest

If you want updates as they drop, **star the repo and turn on notifications/watch it** — releases are frequent and you do not want to miss out.

---

## What TruancyCraftPE already has

Current work in this repository includes:

- **Classic and infinite worlds**
  - classic finite 256x256 worlds
  - infinite terrain with chunk streaming
- **Expanded world generation settings**
  - caves toggle
  - ravines toggle
  - water lakes toggle
  - lava lakes toggle
  - water springs toggle
  - lava springs toggle
  - option to keep a world fully **classic**
- **Improved settings/options work**
  - render distance option
  - more menu and settings work than stock 0.6.1
  - mobile-oriented tuning options like pixels-per-millimeter
- **Multiplayer and LAN work**
  - LAN discovery
  - saved server list support
  - Add Server flow
  - touch Join Game improvements
- **Modern Android build support**
- **Touch/UI cleanup for newer phones**
- **Sprinting**
- **Debug/F3-style information improvements**
- **Ongoing bug fixes across the old codebase**

---

## Terrain and worldgen status

The codebase includes support for modernized infinite-world generation pieces such as:

- biome-based terrain generation
- large cave generation
- canyon / ravine generation
- water lakes
- lava lakes
- configurable cave/ravine/lake/spring generation for world creation

If you want the old feel, you can still make a **Classic world**.
If you want expanded terrain, you can make an **Infinite world** and tune the worldgen options.

---

## Multiplayer status

TruancyCraftPE already includes multiplayer restoration work such as:

- LAN game discovery
- Join Game screen updates
- saved server support
- Add Server UI
- server MOTD ping display for saved/discovered servers
- touch multiplayer UX improvements

This is still an active area of development, especially around compatibility and protocol cleanup.

---

## What we are building toward

Our long-term goal is not just to preserve 0.6.1, but to turn it into the best version of itself.

That means work toward:

- better graphics
- better terrain
- better NPCs / mobs / world behavior
- better controls
- better movement feel
- better camera / looking-around feel
- better UI and usability
- better multiplayer
- better controller support
- better performance
- better modding/restoration infrastructure
- continued bug fixing as we dig through this very old game

This is an old game, and a lot of the work is exactly what you would expect from reviving one:
**fix bugs, modernize carefully, keep the feel, repeat**.

---

## Project goals

TruancyCraftPE exists to:

- preserve the original 0.6.1 alpha experience
- keep classic worlds playable and recognizable
- add optional improvements without forcing the game to lose its identity
- make the game work well on modern Android devices and modern systems
- improve multiplayer and LAN support
- expand world generation while still allowing classic behavior
- give the community a real place to collaborate on old MCPE restoration

---

## Contributing

If you care about classic MCPE, you are welcome here.

You can help by:

1. opening issues for bugs and regressions
2. suggesting gameplay, UI, worldgen, or multiplayer improvements
3. testing APKs and reporting what works and what breaks
4. sending pull requests with focused changes

Please include logs, screenshots, device info, or reproduction steps whenever possible.

---

## Building

### Android build

The current Android build flow uses `build.sh`.

Example full build:

```bash
ANDROID_SDK_ROOT=/home/Alkaline/Android/Sdk \
ANDROID_NDK_PATH=/home/Alkaline/Android/Sdk/ndk/26.1.10909125 \
ADB=/usr/bin/true \
bash ./build.sh --abi arm64-v8a
```

Useful variants:

```bash
# skip Java rebuild
bash ./build.sh --abi arm64-v8a --no-java

# skip C++ rebuild
bash ./build.sh --abi arm64-v8a --no-cpp

# package only
bash ./build.sh --abi arm64-v8a --no-java --no-cpp
```

### Important packaging note

On this project, the Android `d8` / `r8` step can crash on:

- `build-apk/classes/com/mojang/minecraftpe/MainActivity$1.class`

If that happens, the current working fallback is:

```bash
/home/Alkaline/Android/Sdk/build-tools/30.0.3/dx \
  --dex --output=build-apk/classes.dex build-apk/classes

ANDROID_SDK_ROOT=/home/Alkaline/Android/Sdk \
ANDROID_NDK_PATH=/home/Alkaline/Android/Sdk/ndk/26.1.10909125 \
ADB=/usr/bin/true \
bash ./build.sh --abi arm64-v8a --no-java --no-cpp
```

---

## Repository

Active GitHub home:

- https://github.com/Trauncy209/TruancyCraftPE

---

## Final note

TruancyCraftPE is for people who still love old Pocket Edition and want to make it better together.

If you want to help build a stronger, cleaner, community-driven version of classic MCPE, you're in the right place.
