![The Sonic Adventure DX logo with the "Director's Cut" text replaced with "Multiplayer"](/docs/images/logo.png)

# 🏆 Sonic Adventure DX: Multiplayer

*Sonic Adventure DX: Multiplayer* is a heavy modification of Sonic Adventure DX using the [SADX Mod Loader](https://github.com/X-Hax/sadx-mod-loader) that adds local and online multiplayer gameplay. It adds a full-fledged menu, a split-screen mode and netplay capability. A significant amount of objects and systems were patched to work with multiplayer and netplay.

## 📥 How to install

You need the following pre-requisites:
* The [SADX Mod Loader](https://github.com/X-Hax/sadx-mod-loader) ([installer version](https://sadxmodinstaller.unreliable.network/))
* The [Microsoft Visual C++ Redistributable for Visual Studio 2019 (**x86**)](https://aka.ms/vs/16/release/vc_redist.x86.exe) (auto-installed by the installer)

To install this mod, download the mod archive from [here](https://github.com/kellsnc/sadx-multiplayer/releases/latest) and drop the contained folder in your mods folder. Then, enable the mod through the Mod Manager.

Everything can be accessed through the ingame menu. You can use the Mod Manager to configure the default netplay address and port.

## 🛠 How to build:

You need the following pre-requisites on Windows:
* Visual Studio 2019/2022 with the v142 toolkit and 10.0 Windows SDK
* The [DirectX 8.1 SDK](https://archive.org/details/dx81sdk_full)

Simply pull the repository and its submodules recursively, and build the solution.
