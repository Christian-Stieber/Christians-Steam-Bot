# Christians-Steam-Bot

This is a simple project using my Steam-Framework.

Basically, a console-only bot that makes most of the current Framework available:
* handling sale events (discovery queue and stickers)
* "playing" games -- no automated farming yet
* sending inventories to another bot account
* accepting/cancelling trades
* some account information
* card farming

# Building

## Linux

Requirements are listed over at the [Christians-Steam-Frameworkt](https://github.com/Christian-Stieber/Christians-Steam-Framework) repository.

Just run something like `cmake -S . -B build/Debug -D CMAKE_BUILD_TYPE=Debug` to prepare the build environment, and `cmake --build build/Debug` to make an actual build. The result will be `build/Debug/ChristiansSteamBot`.

## Windows

This is designed for Visual Studio 2022 Community Edition, with activated `vcpkg` support.
If `vcpkg` has not been integrated into your VS installation yet, open a Visual Studio Command Prompt and run `vcpkg integrate install`.

With this, just open the main folder. Visual Studio should see the `CMakeLists.txt` there and use it automatically. It will also download and build the required dependecies.

# Usage

[Usage](Docs.md)
