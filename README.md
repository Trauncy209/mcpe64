# TruancyCraftPE

A community-driven restoration and improvement project for **Minecraft Pocket Edition 0.6.1 alpha**.

Our goal is simple: **remake Minecraft PE 0.6.1 the best way possible** while preserving the original feel that made early Pocket Edition special.

We want this repository to be a place where:
- **players** can help test, report issues, and shape the experience
- **developers** can improve performance, controls, rendering, multiplayer, UI, and platform support
- **fans of classic Minecraft PE** can help preserve an important piece of Minecraft history

> Players and developers are both welcome here.
> If you care about classic MCPE, you are part of the mission.

## Mission
This project exists to:
- preserve the original **0.6.1 alpha** experience
- make it run well on modern hardware
- improve controls and usability without losing the old-school feel
- restore unfinished or broken features where it makes sense
- build an open community around classic Minecraft PE modding and restoration

## Project Priorities
Current focus areas include:
- Android compatibility and packaging
- save reliability on modern devices
- options/menu improvements
- controller and non-touch input support
- multiplayer/LAN stability
- rendering fixes and performance
- screen/UI cleanup for newer phones

## Current Status
This repo already includes work on:
- Android build support
- touch control improvements
- fog fixes
- sound fixes
- better debug/F3 information
- sprinting
- chat/commands work in progress
- options/menu work in progress

Still actively being improved:
- controller support
- server hosting and multiplayer polish
- performance optimization
- screen/layout fixes
- modern Android compatibility

## Contributing
We welcome contributions from both experienced developers and first-time contributors.

If you want to help:
1. open an issue for bugs, ideas, or feature requests
2. fork the repo and make focused changes
3. submit a pull request with a clear description
4. include screenshots, logs, or testing notes when relevant

Please read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request.

## Community
A dedicated community space is planned soon.

For now, this repository is the home base for:
- development progress
- bug reports
- feature planning
- testing coordination

When the Discord is ready, it can be added here as the main live community hub.

## How to Build
### Android
Download Android NDK r14b and run `build.ps1`:

```powershell
# Full build (NDK + Java + APK + install)
C:\apkbuild\build.ps1

# Skip NDK recompile (Java/assets changed only)
C:\apkbuild\build.ps1 -NoJava

# Skip Java recompile (C++ changed only)
C:\apkbuild\build.ps1 -NoCpp

# Only repackage + install (no recompile at all)
C:\apkbuild\build.ps1 -NoBuild
```

## Project Home
The active GitHub home for this restoration is:
- https://github.com/Trauncy209/TruancyCraftPE

This build continues earlier community restoration work.

Thanks to everyone helping preserve and improve classic Minecraft PE.
