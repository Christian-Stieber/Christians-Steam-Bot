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

### Visual Studio 2022 Community Edition

This is designed for Visual Studio 2022 Community Edition, with suitable features installed ("Tools" menu, "Get Tools and Features..."). I'm not listing all the optional packages that I'm seeing on each workload, just the ones that I think might not be selected by default.

* Workloads
  * Desktop development with C++
    * C++ CMake tools for Windows
    * vcpkg package manager
  * Windows application development

In addition, you need to activate the `vcpkg` integration with Visual Studio. Open a "Visual Studio Command Prompt" from the "Tools" menu, and run `vcpkg integrate install`.

With everything set up correctly, when you start Visual Studio, select "Open a local folder" to open the top level folder. Visual Studio should detect the `CMakeLists.txt` and start creating the build system automatically, as well as download and compile Boost, OpenSSL and ProtoBuf. This may take a while, but these things are cached. You can also trigger the build system generation manually from the "Project" menu.

With the build system in place, just "Build all" from the "Build" menu. The executable will be `out\build\windows-x64-debug\ChristiansSteamBot.exe`.

For a command line build, refer to [`.github/workflows/build-windows.yaml`](https://github.com/Christian-Stieber/Christians-Steam-Bot/blob/main/.github/workflows/build-windows.yaml).

In order for CPack to work, you'll need to install the [WiX v3 toolset](https://github.com/wixtoolset/wix3/releases/) and the [Visual Studio 2022 Extension](https://marketplace.visualstudio.com/items?itemName=WixToolset.WixToolsetVisualStudio2022Extension)

### Visual Studio Code, MSYS2 etc.

None of these are supported; you're on your own.

# Usage

[Usage](Docs.md)
