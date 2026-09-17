# CODPatch
#### Read my XUID/DemonWare research report [here](https://github.com/sirenuf/COD-PS3-DemonWare-Research)

> [!CAUTION]
> As of right now, **CFW support is only supported** and this will **not work properly on HEN**.
>
> **HEN support will be added in a bit**, but please keep this in mind and **don't install on HEN** until this is updated.
>
> If you've already accidentally installed CODPatch on HEN, restart your PS3 with a FAT32 formatted USB drive and an empty `boot_plugins.txt` folder in the root of the drive.
> Make sure that USB drive is plugged in when you "Enable HEN"
> 
> Then delete the `/dev_hdd0/plugins/CODPatch` folder of your PS3. You can now restart and enable HEN without the USB drive.
>
> **Once HEN support has been added and fully verified and tested, this caution is getting removed. You will then be able to run CODPatch flawlessly on HEN.**


### Patched games:
1. ✅ **Modern Warfare 2**
   *  **Fixes** the underlying XUID missmatch. Stats now save.

2. ✅ **Modern Warfare 3**
   * **Fixes** the underlying XUID missmatch. Stats save like usual and you can play the game like normal. This will also not crash other’s games anymore if the host is an unpatched client with an old account.
   * **Fixes** unpatched player's of new accounts to also join your server and not crash it.

4. ✅ **Black Ops 1**
   * **Fixes** the underlying XUID missmatch. Stats now save.

5. ✅ **Black Ops 2**
   * **Fixes** crashing if you're logged in on PSN while starting the game.

  
### 🔜 Soon™:
* **Universal RCE/RME protection from IW3 to T6**.
* **Auto updater in the background which applies updates on reboot.**
* **XMB setting integration** (it won't be noticeable or break design languages).
* **Unlock All payloads** - after XMB integration is done, you will be able to do unlock all in any game if you want.
* **Easier installation** - more seamless and quick automatic installation method will be done, in favour of end users.

<br>

# Install
**Download** the latest .7z release of CODPatch [**here**](https://github.com/sirenuf/CODPatch-PS3/releases/latest/download/CODPatch.7z).

<br>

* **Install** by dragging the `CODPatch` folder inside the archive into the `/dev_hdd0/plugins` folder off your PS3.

<br>

Afterwards, the file locations should look like this:
* `/dev_hdd0/plugins/CODPatch/CODPatch.sprx`
* `/dev_hdd0/plugins/CODPatch/Modules/Loader.sprx`

<br>

**Then add** the line `/dev_hdd0/plugins/CODPatch/CODPatch.sprx` in your `/dev_hdd0/boot_plugins.txt` file and **reboot** your Playstation 3.

<br>

**After this** CODPatch will **start** and run in the background automatically on every boot, and fix all of the targetted CODs when you start them, automatically.

No static patching or binary redistribution required.


<br>

# Building
### Still a big TODO. If you have problems report them as an issue

<br>

## Prerequisite 
* You **need** to have the official **Sony SDK** installed.
* **Windows only**, unfortunately for now. I couldn't get [PS3DK](https://github.com/FirebirdTA01/PS3DK) setup to work properly with SPRXs that have their own entrys. And PS3DK is explicitly only tested on RPCS3, very unfortunate, might revisit in the future. Sony SDK only supports C++11 on proprietary SN cross compiler, only available for Windows.
* **Visual Studio Community or better** (no cmake with PS3DK)

<br>

1. **Clone with recursion** - needs to be done to grab submodules
```bash
git clone https://github.com/sirenuf/CODPatch-PS3.git --recursive
```
2. **Build scetool** - if you want to use this on HEN, you need to build using the `Release` preset, which gives you signed output files.
```bash
cd CODPatch-PS3
cd vendor/scetool
make
```
3. **Install libpsutil**
```
still wip because the pre-compiled release hasn't been updated with my memory leak fixes yet. Build the program instead of using the Releases.
https://github.com/skiff/libpsutil

in the future i'll probably just redirect to my own fork.
```
4. **Build away**
If libpsutil is setup correctly, you can now build the project by opening the solution in any Visual Studio release.

<br>

# Credits
* [**setsid**](https://github.com/setsid) - For his amazing research in both BO2 and MW3 which helped a ton. His specific BO2 NOP patch is used in this project and works perfectly. **Please check out his project [ps3-tools](https://github.com/setsid/ps3-tools), it is a great resource similar to this project that contains a lot of information related to patches for COD games and other QoS fixes related to the PS3!**

* [**Jacob Schroeder**](https://github.com/jacob-schroeder) - This project wouldn't be what it is now if it wasn't for his initial finding on how DemonWare returns your actual XUID when you authenticate and how to solve it by swapping cache. I had no idea how these engines initially worked and he taught me. Please check out [**IW4Studio**](https://github.com/jacob-schroeder/IW4Studio) and [**IW4-Binaries**](https://github.com/jacob-schroeder/IW4-Binaries)

* [My friend **yausent**](https://github.com/Yausent) - For his patience and help when developing this project.

* And credit to anyone who I've missed in mentioning here
