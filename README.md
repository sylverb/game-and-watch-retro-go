![](assets/gnw.gif)

# Support
You can support my (sylverb) work and get beta versions of new emulators on https://www.patreon.com/sylverb

# NewUI howto
Please run `make help` to see more information.

最新 Game Genie 以及基于 Game Genie 基础之上的 PCE Rom Patcher 已经可以支持中文化。从 [https://github.com/olderzeus/game-genie-codes-nes/tree/zh_cn](https://github.com/olderzeus/game-genie-codes-nes/tree/zh_cn) 可以查阅或自定义相关文件。

:exclamation::exclamation::exclamation: 任何时候都 **不要** 连接 stlink 的 VCC 和 Game & Watch 主板的 VDD 脚 :exclamation::exclamation::exclamation:

一般情况下只需要连接 GND，SWDIO，SWCLK 3个针脚即可。

来自[apple524](https://github.com/apple524)的 ubuntu 刷机攻略：
[Game & Watch 刷机完全教程](https://apple524.github.io/2022/04/03/g-w-hack/)

來自[maxxkao](http://maxxkao.blogspot.com/)的刷機攻略：

[Nintendo Game & Watch 刷機完全攻略](https://maxxkao.blogspot.com/2022/01/game-watch.html)

[Zelda Game & Watch Dual System 薩爾達機刷雙系統完全攻略](https://maxxkao.blogspot.com/2022/02/zelda-game-watch-dual-boot-wip.html)

## **Undocumented 256k BANK used by default**
 Now NewUI used BANK as undocumented size 256k by default, It's required [patched version of openocd](https://github.com/kbeckmann/ubuntu-openocd-git-builder) to work. You can set `BIG_BANK=0` disabled it to use 128k BANK size.

:exclamation: 128k internal is too small to enough Coverflow UI | i18n | multi font and program more game rom, So [patched version of openocd](https://github.com/kbeckmann/ubuntu-openocd-git-builder) is recommend.

### i18n support
`make CODEPAGE=lang` Sets codepage to configures default display language.(default=1252 as English);
Expert `UICODEPAGE=lang` set the ui display language, default `UICODEPAGE=CODEPAGE`;
- 1252 : English
- 936 : 简体中文
- 950 : 繁體中文
- 949  : 한국어 (translation by [Augen(히힛마스터)](https://github.com/#))
- 12521 : Español (translation by [Icebox2](https://github.com/Icebox2))
- 12522 : Portuguese (translation by [DefKorns](https://github.com/DefKorns))
- 12523 : Français (translation by [Narkoa](https://github.com/Narkoa))
- 12524 : Italiano (translation by [SantX27](https://github.com/SantX27))
- 12521 : Russian (translation by [teuchezh](https://github.com/teuchezh))
- 12525 : German (translation by [LeZerb](https://github.com/LeZerb))
- 932 : 日本語(translation by [macohime](https://github.com/macohime))

You can set `[OPT]=[0|1]` to include or exclude some language, List of `OPT` parameter:
`EN_US`  English; `ES_ES`  Español; `PT_PT` Portuguese; `FR_FR` Français; `IT_IT` Italiano; `ZH_CN` 简体中文; `ZH_TW` 繁體中文;`KO_KR` 한국어; `JA_JP` 日本語; `DE_DE` Deutsch; `RU_RU` Russian;

other : Wait your support to translate

### Coverflow support
`make COVERFLOW=1 JPG_QUALITY=(90)`  set `COVERFLOW=1` to support show cover art. support `.png` `.bmp` `.jpg` file which filename same as rom's filename and same floder. CFW will convet it to jpg format file `.img` and pack it into firmware, you can custom jpg's quality use `JPG_QUALITY`.

### Other features
Here some other features you can edit the rom define file to custom by youself.
Before you run `make flash`, please run `make romdef` then you can get some romdef file in `roms` folder as `gb.json` `nes.json` ..etc. Edit this files then you can custom the follow feature.

Use `make ROMINFOCODE=[ascii|?]` to set charset of rominfo sourcecode to enabled local language support.

- Game display title(Set `name` value, title's charset be must your custom language supported)
- Pack or don't pack rom to firmware (Set `publish` to `1` or `0`)
- Emulator system cover image size(Set `_cover_width` and `_cover_height`, 180>=`_cover_width`>=64 and 136>=`_cover_height`>=64)

`make romdef` is run patched mode for `*emu*.json` if the file already exist, each time only append new rom's information when command execute.


***********
# Emulator collection for Nintendo® Game & Watch™

This is a port of the [retro-go](https://github.com/ducalex/retro-go) emulator collection that runs on the Nintendo® Game & Watch™: Super Mario Bros. system.

Supported emulators:

- Amstrad CPC6128 *beta* (amstrad) (check [Amstrad CPC6128 Emulator](#amstrad-cpc6128-emulator) section for details about the Amstrad CPC6128 emulator)
- Atari 7800 (a7800)
- ColecoVision (col)
- Gameboy / Gameboy Color (gb/gbc)
- Game & Watch / LCD Games (gw)
- MSX1/2/2+ (msx) (check [MSX Emulator](#msx-emulator) section for details about MSX emulator)
- Nintendo Entertainment System (nes) (check [NES Emulator](#nes-emulator) section for details about NES emulator)
- PC Engine / TurboGrafx-16 (pce)
- Sega Game Gear (gg)
- Sega Genesis / Megadrive (md)
- Sega Master System (sms)
- Sega SG-1000 (sg)
- Videopac / Odyssey2 (videopac)
- Watara Supervision (wsv)

Supported SNES game _ports_:

- The Legend of Zelda: A Link to the Past
- Super Mario World

Supported Homebrew _ports_:

- ccleste (a port of Pico-8 version of Celeste Classic)

## Table of Contents
- [NewUI howto](#newui-howto)
  - [**Undocumented 256k BANK used by default**](#undocumented-256k-bank-used-by-default)
    - [i18n support](#i18n-support)
    - [Coverflow support](#coverflow-support)
    - [Other features](#other-features)
    - [FONT](#font)
- [Emulator collection for Nintendo® Game & Watch™](#emulator-collection-for-nintendo-game--watch)
  - [Table of Contents](#table-of-contents)
  - [Controls](#controls)
    - [Macros](#macros)
  - [Troubleshooting / FAQ](#troubleshooting--faq)
  - [How to build](#how-to-build)
    - [Prerequisites](#prerequisites)
    - [Building](#building)
    - [Information for developers](#information-for-developers)
  - [Build and flash using Docker](#build-and-flash-using-docker)
  - [Backing up and restoring save state files](#backing-up-and-restoring-save-state-files)
  - [Screenshots](#screenshots)
  - [Compression support](#compression-support)
  - [Cheat codes](#cheat-codes)
    - [Cheat codes on NES System](#cheat-codes-on-nes-system)
    - [Cheat codes on GB System](#cheat-codes-on-gb-system)
    - [Cheat codes on PCE System](#cheat-codes-on-pce-system)
    - [Cheat codes on MSX System](#cheat-codes-on-msx-system)
  - [Upgrading the flash](#upgrading-the-flash)
  - [Advanced Flash Examples](#advanced-flash-examples)
    - [Custom Firmware (CFW)](#custom-firmware-cfw)
  - [SNES game ports](#snes-game-ports)
    - [The Legend of Zelda: A Link to the Past](#the-legend-of-zelda-a-link-to-the-past)
    - [Super Mario World](#super-mario-world)
  - [Homebrew ports](#homebrew-ports)
    - [Celeste Classic](#celeste-classic)
  - [Discord, support and discussion](#discord-support-and-discussion)
  - [LICENSE](#license)

## Controls

Buttons are mapped as you would expect for each emulator. `GAME` is mapped to `START`,
and `TIME` is mapped to `SELECT`. `PAUSE/SET` brings up the emulator menu.

By default, pressing the power-button while in a game will automatically trigger
a save-state prior to putting the system to sleep. Note that this WILL overwrite
the previous save-state for the current game.

### Macros

Holding the `PAUSE/SET` button while pressing other buttons have the following actions:

| Button combination    | Action                                                                 |
| --------------------- | ---------------------------------------------------------------------- |
| `PAUSE/SET` + `GAME`  | Store a screenshot. (Disabled by default on 1MB flash builds)          |
| `PAUSE/SET` + `TIME`  | Toggle speedup between 1x and the last non-1x speed. Defaults to 1.5x. |
| `PAUSE/SET` + `UP`    | Brightness up.                                                         |
| `PAUSE/SET` + `DOWN`  | Brightness down.                                                       |
| `PAUSE/SET` + `RIGHT` | Volume up.                                                             |
| `PAUSE/SET` + `LEFT`  | Volume down.                                                           |
| `PAUSE/SET` + `B`     | Load state.                                                            |
| `PAUSE/SET` + `A`     | Save state.                                                            |
| `PAUSE/SET` + `POWER` | Poweroff WITHOUT save-stating.                                         |

## Troubleshooting / FAQ

- Run `make help` to get a list of options to configure the build, and targets to perform various actions.
- Do you have any changed files, even if you didn't intentionally change them? Please run `git reset --hard` to ensure an unchanged state.
- Did you run `git pull` but forgot to update the submodule? Run `git submodule update --init --recursive` to ensure that the submodules are in sync or run `git pull --recurse-submodules` instead.
- Run `make clean` and then build again. The makefile should handle incremental builds, but please try this first before reporting issues.
- If you have limited resources on your computer, remove the `-j$(nproc)` flag from the `make` command, i.e. run `make flash`.
- If you have changed the external flash and are having problems:
  - If your chip was bought from e.g. ebay, aliexpress or similar places, you might have gotten a fake or bad clone chip. You can set `EXTFLASH_FORCE_SPI=1` to disable quad mode which seems to help for some chips.
- It is still not working? Try the classic trouble shooting methods: Disconnect power to your debugger and G&W and connect again. Try programming the [Base](https://github.com/ghidraninja/game-and-watch-base) project first to ensure you can actually program your device.
- Still not working? Ok, head over to #support on the discord and let's see what's going on.

## How to build

### Prerequisites

- You will need version 10 or later of [arm-gcc-none-eabi toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads). **10.2.0 and later are known to work well**. Please make sure it's installed either in your PATH, or set the environment variable `GCC_PATH` to the `bin` directory inside the extracted directory (e.g. `/opt/gcc-arm-none-eabi-10-2020-q4-major/bin`, `/Applications/ARM/bin` for macOS).
- [GnWManager](https://github.com/BrianPugh/gnwmanager) for flashing firmware and managing the filesystem.
- In order to run this on a Nintendo® Game & Watch™ [you need to first unlock it](https://github.com/ghidraninja/game-and-watch-backup/).

### Building

Note: `make -j8` is used as an example. You may use `make -j$(nproc)` on Linux or `make -j$(sysctl -n hw.logicalcpu)` on Mac, or just write the number of threads you want to use, e.g. `make -j8`.

```bash
# Configure the debug adapter you want to use.
# stlink is also the default, but you may set it to something else:
# export ADAPTER=jlink
# export ADAPTER=rpi
export ADAPTER=stlink

# Clone this repo with submodules:

git clone --recurse-submodules https://github.com/sylverb/game-and-watch-retro-go

cd game-and-watch-retro-go

# Install python dependencies
python3 -m pip install -r requirements.txt

# Place roms in the appropriate folders:
# cp /path/to/rom.gb ./roms/gb/
# cp /path/to/rom.nes ./roms/nes/
# etc. for each rom-emulator combination.

# On a Mac running make < v4 you have to manually download the HAL package by running:
# make download_sdk

# Build and program external and internal flash.
# Notes:
#     * If you are using a modified unit with a larger external flash,
#       set the EXTFLASH_SIZE_MB to its size in megabytes (MB) (16MB used in the example):
#           make -j8 EXTFLASH_SIZE_MB=16 flash
#     * If you have the Zelda version you can set GNW_TARGET=zelda to have the appropriate
#       flash size and theme set. If you want to stick with the red theme you can set
#       EXTFLASH_SIZE_MB=4 on your Zelda model.

make -j8 flash
```

### Information for developers

If you need to change the project settings and generate c-code from stm32cubemx, make sure to not have a dirty working copy as the tool will overwrite files that will need to be perhaps partially reverted. Also update Makefile.common in case new drivers are used.

## Build and flash using Docker

<details>
  <summary>
    If you are familiar with Docker and prefer a solution where you don't have to manually install toolchains and so on, expand this section and read on.
  </summary>

  To reduce the number of potential pitfalls in installation of various software, a Dockerfile is provided containing everything needed to compile and flash retro-go to your Nintendo® Game & Watch™: Super Mario Bros. system. This Dockerfile is written tageting an x86-64 machine running Linux.

  Steps to build and flash from a docker container (running on Linux, e.g. Archlinux or Ubuntu):

  ```bash
  # Clone this repo
  git clone --recursive https://github.com/sylverb/game-and-watch-retro-go

  # cd into it
  cd game-and-watch-retro-go

  # Place roms in the appropriate directory inside ./roms/

  # Build the docker image (takes a while)
  make docker_build

  # Run the container.
  # The current directory will be mounted into the container and the current user/group will be used.
  # In order to be able to flash the device, the container is started with --priviliged and also mounts
  # in /dev/bus/usb. See Makefile.common for the exact command line that is executed if curious.
  make docker

  # Build and flash from inside the container:
  docker@76f83f2fc562:/opt/workdir$ make ADAPTER=stlink EXTFLASH_SIZE_MB=1 -j$(nproc) flash
  ```

</details>

## Backing up and restoring save state files

Save states can be backed up by running `make flash_saves_backup`.

:exclamation: Note that `INTFLASH_BANK` or `INTFLASH_ADDRESS` must be the same value as when initially flashed. This is best done with `export VARIABLE=value`. Usually, if you only have retro-go installed, then this doesn't need to be specified. If you are dual-booting, then you probably want to specify `INTFLASH_BANK=2`.

This downloads all save states to the local directory `./backup/savestate`. Each save state will be located in `./save_states/<emu>/<rom name>/0`.

After this, it is safe to change roms, pull new code and build & flash the device. If just flashing new firmware/roms while keeping other parameters the same, the save states on-device will persist.

Save states can be restored by running `make flash_saves_restore`. Again, `INTFLASH_BANK` or `INTFLASH_ADDRESS` must be set appropriately.

You can also erase the filesystem (and thusly all saves) by running `make format` while the device is on.

## Screenshots

Screenshots can be captured by pressing `PAUSE/SET` + `GAME`.

Screenshots can be downloaded by running `make dump_screenshot`, and will be saved as a 24-bit RGB PNG to `./screenshot.png`.

## Compression support

Default configuration when building a firmware is to compress files when possible. The compression is done in different ways depending on each emulator.
General case is that we will be able to compress roms that can be decompressed in ram, for example if there is more than 512kB of ram free with a given emulator, then we will be able to compress roms up to 512kB. Larger roms will be stored in flash without compression. Here is the maximum size for compressed roms for different systems :
- Atari 7800 : 128kB
- MSX : 128kB
- Nintendo NES/Famicom : 512kB
- PC Engine : 292kB
- SG1000/Colecovision : 60kB
- Watara Supervision : 512kB

Some systems are handling roms as pages of data, so it's possible to compress roms data following this page organization so the emulator will decompress a page of data on request. A caching system is often in place to prevent decompressing (often used) pages too often (as decompressing data is taking time). This is also possible with disks games as they are made of sides/tracks/sectors ! The following systems have no limit in the size of compressed roms thanks to that :
- Amstrad CPC6128
- Nintendo Gameboy/Gameboy Color
- MSX (disks and HDD images only, roms compression is possible up to 128kB)

Some systems are not supporting compression due to the lack of free ram (SMS/GG/Genesis), what could be possible is to decompress a rom and store it in an unused zone of flash, the only problem with that would be the wearing of flash chip blocks (a flash memory block has an erase/flash count limitation, typical value is 10 000 times).

## Cheat codes

Note: Currently cheat codes are only working with NES, PCE and MSX games.

To enable, add CHEAT_CODES=1 to your make command. If you have already compiled without CHEAT_CODES=1, I recommend running make clean first.
To enable or disable cheats, select a game then select "Cheat Codes". You will be able to select cheats you want to enable/disable. Then you can start/resume a game and selected cheats will be applied.
On MSX system, you can enable/disable cheats during game.

### Cheat codes on NES System

To add Game Genie codes, create a file ending in .ggcodes in the same directory as your rom with the same name as your rom. For instance, for
"roms/nes/Super Mario Bros.nes" make a file called "roms/nes/Super Mario Bros.ggcodes". In that file, each line can have up to 3 Game Genie codes and a maximum
of 16 lines of active codes (for a max of 3 x 16 = 48 codes). Each line can also have a description (up to 25 characters long).
You can comment out a line by prefixing with # or //. For example:
```
SXIOPO, Inf lives
APZLGG+APZLTG+GAZUAG, Mega jump
YSAOPE+YEAOZA+YEAPYA, Start on World 8-1
YSAOPE+YEAOZA+LXAPYA, Start on World -1
GOZSXX, Invincibility
# TVVOAE, Circus music
```
When you re-flash, you can enable / disable each of your codes in the game selection screen.

A collection of codes can be found here: [https://github.com/martaaay/game-and-watch-retro-go-game-genie-codes](https://github.com/martaaay/game-and-watch-retro-go-game-genie-codes).

### Cheat codes on GB System

To add Game Genie/Game Shark codes, create a file ending in .ggcodes in the same directory as your rom with the same name as your rom. For instance, for
"roms/gb/Wario Land 3.gb" make a file called "roms/gb/Wario Land 3.ggcodes". In that file, each line can have several Game Genie / Game Shark codes
(separate them using a +) and a maximum of 16 lines of active codes. Each line can also have a description (up to 25 characters long).
You can comment out a line by prefixing with # or //. For example:
```
SXIOPO, Inf lives
APZLGG+APZLTG+GAZUAG, Mega jump
YSAOPE+YEAOZA+YEAPYA, Start on World 8-1
YSAOPE+YEAOZA+LXAPYA, Start on World -1
GOZSXX, Invincibility
# TVVOAE, Circus music
```
When you re-flash, you can enable / disable each of your codes in the game selection screen or during game.

### Cheat codes on PCE System

Now you can define rom patch for PCE Roms. You can found patch info from [Here](https://krikzz.com/forum/index.php?topic=1004.0).
To add PCE rom patcher, create a file ending in .pceplus in the same directory as your rom with the same name as your rom. For instance, for
"roms/pce/1943 Kai (J).pce" make a file called "roms/pce/1943 Kai (J).pceplus".
A collection of codes file can be found [here](https://github.com/olderzeus/game-genie-codes-nes/tree/master/pceplus).

Each line of pceplus is defined as:
```
01822fbd,018330bd,0188fcbd,	Hacked Version
[patchcommand],[...], patch desc

```

Each patch command is a hex string defined as:
```
01822fbd
_
|how much byte to patched
 _____
   |patch start address, subtract pce rom header size if had.
      __...
       |bytes data to patched from start address

```
### Cheat codes on MSX System

You can use blueMSX MCF cheat files with your Game & Watch. A nice collection of patch files is available [Here](http://bluemsx.msxblue.com/rel_download/Cheats.zip).
Just copy the wanted MCF files in your roms/msx folder with the same name as the corresponding rom/dsk file.
On MSX system, you can enable/disable cheats while playing. Just press the Pause/Set button and choose "Cheat Codes" menu to choose which cheats you want to enable or disable.

## Upgrading the flash

The Nintendo® Game & Watch™ comes with a 1MB external flash. This can be upgraded.

The flash operates at 1.8V so make sure the one you change to also matches this.

The recommended flash to upgrade to is MX25U12835FM2I-10G. It's 16MB, the commands are compatible with the stock firmware and it's also the largest flash that comes in the same package as the original.

:exclamation: Make sure to backup and unlock your device before changing the external flash. The backup process requires the external flash to contain the original data.

## Advanced Flash Examples

### Custom Firmware (CFW)
In order to install both the CFW (modified stock rom) and retro-go at the same time, a [patched version of openocd](https://github.com/kbeckmann/ubuntu-openocd-git-builder) needs to be installed and used.

In this example, we'll be compiling retro-go to be used with a 64MB (512Mb) `MX25U51245GZ4I00` flash chip and [custom firmware](https://github.com/BrianPugh/game-and-watch-patch). The internal custom firmware will be located at `0x08000000`, which corresponds to `INTFLASH_BANK=1`. The internal retro-go firmware will be flashed to `0x08100000`, which corresponds to `INTFLASH_BANK=2`. The configuration of custom firmware described below won't use any extflash, so no `EXTFLASH_OFFSET` is specified. We can now build and flash the firmware with the following command:

```bash
make clean
make -j8 EXTFLASH_SIZE_MB=64 INTFLASH_BANK=2 flash
```

To flash the custom firmware, [follow the CFW README](https://github.com/BrianPugh/game-and-watch-patch#retro-go). But basically, after you install the dependencies and place the correct files in the directory, run:
```bash
# In the game-and-watch-patch folder
make PATCH_PARAMS="--internal-only" flash_patched_int
```

## NES Emulator

NES emulation was done using nofrendo-go emulator, it doesn't have the best compatibility but has good performances.
To handle eveything that is not or badly emulated by nofrendo, fceumm emulator has been ported too.
fceumm has very good compatibility but it's using more CPU power, currently about 65-85% of CPU depending on games, disks games (FDS) are taking even more CPU power (about 95%).
Due to the large amount of mappers supported by fceumm, it's not possible to embbed all mappers codes in the G&W memory, so the parsing proccess is listing mappers used by games in the rom/nes folder so only needed mappers are loaded in the G&W. If you are using too much different mappers in the games you have selected, you will have runtime problems. The maximum number of mappers you can use will depends on embedded mappers, but basically it should fit most needs.

Mappers compatibility is basically the same as fceumm version from 01/01/2023. Testing all mappers is not possible, so some mappers that could try to allocate too much ram will probably crash. If you find any mapper that crash, please report on discord support, or by opening a ticket on github.

As Game & Watch CPU is not able to emulate YM2413 at 48kHz, mapper 85 (VRC-7) sound will play at 18kHz instead of 48kHz.

FDS support requires you to put the FDS firmware in `roms/nes_bios/disksys.rom` file

Note that you can force nofrendo-go usage instead of fceumm by adding FORCE_NOFRENDO=1 option in your make command

## MSX Emulator

MSX system is a computer with a keyboard and with multiple extensions possible (like sound cartridges).
The system needs bios files to be in the roms/msx_bios folder. Check roms/msx_bios/README.md file for details.

What is supported :
- MSX1/2/2+ system are supported. MSX Turbo-R will probably not work on the G&W.
- ROM cartridges images : roms have to be named with rom, mx1 or mx2 extension.
- Disks images : disks images have to be named with dsk extension. Due to memory constraints, disks images are read only. Multiple disks games are supported and user can change the current disk using the "Pause/Options/Change Dsk" menu.
- Cheat codes support (MCF files in old or new format as described [Here](http://www.msxblue.com/manual/trainermcf_c.htm))
- The file roms/msx_bios/msxromdb.xml contains control profiles for some games, it allows to configure controls in the best way for some games. If a game has no control profile defined in msxromdb.xml, then controls will be configured as joystick emulation mode.
- Sometimes games require the user to enter his name using the keyboard, and some games like Metal Gear 1/2 are using F1-F5 keys to acces items/radio/... menus. It is possible to virtually press these keys using the "Pause/Options/Press Key" menu.

Note that the MSX support is done using blueMsx 2.8.2, any game that is not working correctly using this emulator will not work on the Game & Watch. To fit in the G&W, a some features have been removed, so it's possible that some games running on blueMSX will not work in the G&W port. The emulator port is still in progress, consider it as a preview version.

## Amstrad CPC6128 Emulator

Amstrad CPC6128 system is a computer with a keyboard and disk drive.

What is supported :
- Amstrad CPC6128 system is the only supported system. CPC464 could be added if there is any interest in doing this. Note that CPC464+/6128+ systems are not supported (running a around 40% of their normal speed so it has been removed)
- Disks images : disks images have to be named with dsk extension. Due to memory constraints, disks images are read only. Multiple disks games are supported and user can change the current disk using the "Pause/Options/Change Dsk" menu.  Both standard and extended dsk format are supported, moreover a compression mechanism specific to the G&W has been implemented to reduce the size of disk images. Disk compression is automatically handled during the build process.
- Normally when the amstrad system starts, it will wait the user to enter a run"file or |CPM command to load the content of the disk. As it's not very friendly, the emulator is detecting the name of the file to run and enter automatically the right commant at startup
- Sometimes games require the user to enter his name using the keyboard. It is possible to virtually press these keys using the "Pause/Options/Press Key" menu.
- Amstrad screen resolution is 384x272 pixels while G&W resolution is 320x240. The standard screen mode (with no scaling) will show the screen without the borders which will be ok in most cases, but in some cases games are using borders to show some content. If you want to see the whole Amstrad screen on the G&W, set options/scaling to "fit".

Tape support has not been ported, if there is any interest in adding this, it could be considered.

Note that the Amstrad CPC6128 support is done using caprice32 emulator, any game that is not working correctly using this emulator will not work on the Game & Watch. To fit in the G&W, a some features have been removed, so it's possible that some games running on caprice32 will not work in the G&W port. The emulator port is still in progress, consider it as a preview version.

## Vectrex/Odyssey2 Emulator
Vectrex/Odyssey2 is provided by a modified version of o2em emulator.
Support is currently in development so it's unstable, has lots of bugs and it's not really playable.
To play, you need a bios file, for now rename your bios file to bios.bin and put it in the roms/vectrex folder

## SNES game ports

Some SNES game have been _ported_ to the G&W.

### The Legend of Zelda: A Link to the Past

To enable this port, copy the SNES ROM (US version) to `roms/zelda3/zelda3.sfc`.

Due to the limited set of buttons (especially on the Mario console), the controls are peculiar:

| Description | Binding on Mario units | Binding on Zelda units |
| ----------- | ---------------------- | ---------------------- |
| `A` button (Pegasus Boots / Interacting) | `A` | `A` |
| `B` button (Sword) | `B` | `B` |
| `X` button (Show Map) | `GAME + B` | `TIME` |
| `Y` button (Use Item) | `TIME` | `SELECT` |
| `Select` button (Save Screen) | `GAME + TIME` | `GAME + TIME` |
| `Start` button (Item Selection Screen) | `GAME + A` | `START` |
| `L` button (Quick-swapping, if enabled) | `-` | `GAME + B` |
| `R` button (Quick-swapping, if enabled) | `-` | `GAME + A` |

Some features can be configured with flags:

| Build flag    | Description |
| ------------- | ------------- |
| `LIMIT_30FPS` | Limit to 30 fps for improved stability.<br>Enabled by default.<br>Disabling this flag will result in unsteady framerate and stuttering. |
| `FASTER_UI` | Increase UI speed (item menu, etc.).<br>Enabled by default. |
| `BATTERY_INDICATOR` | Display battery indicator in item menu.<br>Enabled by default. |
| `FEATURE_SWITCH_LR` | Item switch on L/R. Also allows reordering of items in inventory by pressing Y+direction.<br>Hold X, L, or R inside of the item selection screen to assign items to those buttons.<br>If X is reassigned, Select opens the map. Push Select while paused to save or quit.<br>When L or R are assigned items, those buttons will no longer cycle items. |
| `FEATURE_TURN_WHILE_DASHING` | Allow turning while dashing. |
| `FEATURE_MIRROR_TO_DARK_WORLD` | Allow mirror to be used to warp to the Dark World. |
| `FEATURE_COLLECT_ITEMS_WITH_SWORD` | Collect items (like hearts) with sword instead of having to touch them. |
| `FEATURE_BREAK_POTS_WITH_SWORD` | Level 2-4 sword can be used to break pots. |
| `FEATURE_DISABLE_LOW_HEALTH_BEEP` | Disable the low health beep. |
| `FEATURE_SKIP_INTRO_ON_KEYPRESS` | Avoid waiting too much at the start.<br>Enabled by default. |
| `FEATURE_SHOW_MAX_ITEMS_IN_YELLOW` | Display max rupees/bombs/arrows with orange/yellow color. |
| `FEATURE_MORE_ACTIVE_BOMBS` | Allows up to four bombs active at a time instead of two. |
| `FEATURE_CARRY_MORE_RUPEES` | Can carry 9999 rupees instead of 999. |
| `FEATURE_MISC_BUG_FIXES` | Enable various zelda bug fixes. |
| `FEATURE_CANCEL_BIRD_TRAVEL` | Allow bird travel to be cancelled by hitting the X key. |
| `FEATURE_GAME_CHANGING_BUG_FIXES` | Enable some more advanced zelda bugfixes that change game behavior. |
| `FEATURE_SWITCH_LR_LIMIT` | Enable this to limit the ItemSwitchLR item cycling to the first 4 items. |

#### Alternate languages

By default, dialogues extracted from the US ROM are in english. You can replace dialogues with another language by adding a localized ROM file. Supported alternate languages are:

| Language | Origin | Naming | SHA1 hash |
| -------- | ------ | ------ | --------- |
| German   | Original | zelda3_de.sfc | 2E62494967FB0AFDF5DA1635607F9641DF7C6559 |
| French   | Original | zelda3_fr.sfc | 229364A1B92A05167CD38609B1AA98F7041987CC |
| French (Canada) | Original | zelda3_fr-c.sfc | C1C6C7F76FFF936C534FF11F87A54162FC0AA100 |
| English (Europe) | Original | zelda3_en.sfc | 7C073A222569B9B8E8CA5FCB5DFEC3B5E31DA895 |
| Spanish  | Romhack | zelda3_es.sfc | 461FCBD700D1332009C0E85A7A136E2A8E4B111E |
| Polish   | Romhack | zelda3_pl.sfc | 3C4D605EEFDA1D76F101965138F238476655B11D |
| Portuguese | Romhack | zelda3_pt.sfc | D0D09ED41F9C373FE6AFDCCAFBF0DA8C88D3D90D |
| Dutch    | Romhack | zelda3_nl.sfc | FA8ADFDBA2697C9A54D583A1284A22AC764C7637 |
| Swedish  | Romhack | zelda3_sv.sfc | 43CD3438469B2C3FE879EA2F410B3EF3CB3F1CA4 |

### Super Mario World

To enable this port, copy the SNES ROM (US version) to `roms/smw/smw.sfc`.

Due to the limited set of buttons (especially on the Mario console), the controls are peculiar:

| Description | Binding on Mario units | Binding on Zelda units |
| ----------- | ---------------------- | ---------------------- |
| `A` button (Spin Jump) | `GAME + A` | `SELECT` |
| `B` button (Regular Jump) | `A` | `A` |
| `X`/`Y` button (Dash/Shoot) | `B` | `B` |
| `Select` button (Use Reserve Item) | `TIME` | `TIME` |
| `Start` button (Pause Game) | `GAME + TIME` | `START` |
| `L` button (Scroll Screen Left) | `-` | `GAME + B` |
| `R` button (Scroll Screen Right) | `-` | `GAME + A` |

Some features can be configured with flags:

| Build flag    | Description |
| ------------- | ------------- |
| `LIMIT_30FPS` | Limit to 30 fps for improved stability.<br>Enabled by default.<br>Disabling this flag will result in unsteady framerate and stuttering. |

## Homebrew ports

Some homebrew games have been _ported_ to the G&W.

### Celeste Classic

This is a port of the Pico-8 version of Celeste Classic.
To enable this port, add a `roms/homebrew/Celeste.png` file.

## Discord, support and discussion 

Please join the [Discord](https://discord.gg/vVcwrrHTNJ).

## LICENSE

This project is licensed under the GPLv2. Some components are also available under the MIT license. Respective copyrights apply to each component.
