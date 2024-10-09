# Merging Forge
Forge can be added to various sources by applying the core changes required for Forge to function.  
Below you find the guides for merging Forge with each core or a fork of it.  
If you choose to merge you should be able to maintain and update yourself - we do not maintain your core. View Unofficial Merging below.
We also do not fix any merging errors you may have, but you are free to ask about them on the [support forum](../README.md#documentation) and we may assist.

We recommend using the [installation guide](INSTALL.md) especially if you are not familiar with git and updating the code.  
It allows you to simply use `git pull` followed by `git submodule update` to update your source and we will handle the merging and maintenance with the official core source. Naturally you still need to handle updating the database as instructed by the core's wiki or instructions.

### Merging Forge with MaNGOS
Forge is merged with [official MaNGOS](http://getmangos.eu/) by default.

### Merging Forge with cMaNGOS
```
git clone https://github.com/cmangos/mangos-wotlk.git
cd mangos-wotlk
git pull --recurse-submodules https://github.com/ForgeLuaEngine/ForgeMangosWotlk.git
```
Steps explained:
1. clone the core or fork source or get the it by other means
2. navigate to the source folder
3. pull the Forge fork. This will fetch the repository and merge it with your source.
  * `--recurse-submodules` will automatically pull the submodules (Forge repository). You may need to use `git submodule init` followed by `git submodule update` if your Forge folder is empty
  * it is important that you choose the correct Forge fork for your core andpatch:
    * [Forge cMaNGOS Classic](https://github.com/ForgeLuaEngine/ForgeMangosClassic)
    * [Forge cMaNGOS TBC](https://github.com/ForgeLuaEngine/ForgeMangosTbc)
    * [Forge cMaNGOS WotLK](https://github.com/ForgeLuaEngine/ForgeMangosWotlk)

### Merging Forge with TrinityCore
```
git clone https://github.com/TrinityCore/TrinityCore.git -b3.3.5
cd TrinityCore
git pull --recurse-submodules https://github.com/ForgeLuaEngine/ForgeTrinityWotlk.git
```
Steps explained:
1. clone the core or fork source or get the it by other means
2. navigate to the source folder
3. pull the Forge fork. This will fetch the repository and merge it with your source.
  * `--recurse-submodules` will automatically pull the submodules (Forge repository). You may need to use `git submodule init` followed by `git submodule update` if your Forge folder is empty
  * it is important that you choose the correct Forge fork for your core and patch:
    * [Forge TrinityCore WotLK](https://github.com/ForgeLuaEngine/ForgeTrinityWotlk)
    * [Forge TrinityCore Cataclysm](https://github.com/ForgeLuaEngine/ForgeTrinityCata)
