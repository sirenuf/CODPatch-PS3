# Installation
Be sure to have a COBRA/PS3MAPI ready system, if you use modern HEN or a modern CFW like EVILNAT, then you don't have to worry.

1. Download `CODPatch.sprx` from the releases tab.
2. Place `CODPatch.sprx` inside `/dev_hdd0/tmp/CODPatch.sprx` on your PS3, via FTP (you establish a FTP connection by launching multiMAN on your PS3, or by having webMAN-MOD installed).
3. Look for a file with the location: `/dev_hdd0/boot_plugins.txt`, if it doesn't exist, create it.
4. Edit `/dev_hdd0/boot_plugins.txt` and add a new line: `/dev_hdd0/tmp/CODPatch.sprx` and save the file.
5. Reboot your PS3.

You're now done and your stat will save when playing COD MW2 multiplayer!

FYI: All of your saved data will be saved in the CODPatch directory inside `/dev_hdd0/tmp`, automatically created by CODPatch when you first sign into MW2 multiplayer.

# How to build
You need to include the submodules of this project when cloning in order to compile it:

```bash
git clone https://github.com/sirenuf/CODPatch-PS3.git --recursive
```

## Building
1. Install the latest version of [libpsutil](https://github.com/skiff/libpsutil)
2. Now clone [my fork](https://github.com/sirenuf/libpsutil) which patches issues not yet implemented by libpsutil, including a [memory leak](https://github.com/skiff/libpsutil/commit/437038680d273ab8e3a0e215579e6699e8aad826).
3. Build my fork in Visual Studio. It will produce a `libpsutil.a`. Replace the original that upstream libpsutil comes with, with the built library.
4. Replace libpsutil.h with libpsutil.h included in my fork, inside the include/ directory of the fork.
5. Now to CODPatch, navigate to vendor/scetool and run `make` in your terminal.
6. You can now build CODPatch by opening the solution file and hitting `CTRL + SHIFT + F5` in Visual Studio 

# Credits
TBA
