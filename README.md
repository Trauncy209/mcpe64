# mcpe64

> [!WARNING]  
> This repository has been merged with the original repository and is **no longer actively maintained**.  
> Future work will continue on the original repo: https://gitea.sffempire.ru/Kolyah35/minecraft-pe-0.6.1  
> Join the Discord server for updates and support: https://discord.gg/ryZ884DWJf

Source code for **Minecraft Pocket Edition 0.6.1 alpha** with various fixes and improvements.

This project builds upon the original repository by Kolyah35 and aims to preserve and improve this early version of Minecraft PE.

I've done some bad and pointless changes to the code so if someone could improve my horrible debugging skills I'd be very happy.

## Roadmap
- [x] Fix fog
- [x] Fix sound
- [x] Add sprinting
- [x] Chat (semi working) and commands
- [x] Implementing options 
- [ ] Controller support
- [ ] Minecraft server hosting
- [x] Android build support
    - [x] Touch control improvements
    - [ ] Screen fixes
- [x] Better F3
- [ ] Performance optimalizations

## How to Build
### Android
Download r14b Android NDK and run `build.ps1`:
```
# Full build (NDK + Java + APK + install)
C:\apkbuild\build.ps1

# Skip NDK recompile (Java/assets changed only)
C:\apkbuild\build.ps1 -NoJava

# Skip Java recompile (C++ changed only)  
C:\apkbuild\build.ps1 -NoCpp

# Only repackage + install (no recompile at all)
C:\apkbuild\build.ps1 -NoBuild
```

## Credits
Original repository:  
https://gitea.sffempire.ru/Kolyah35/minecraft-pe-0.6.1
