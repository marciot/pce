
PCE for the RetroWeb Vintage Computer Museum
============================================

This repository contains the JavaScript version of PCE used in the [retroweb-vintage-computer-museum](https://github.com/marciot/retroweb-vintage-computer-museum). This JavaScript port of PCE provides the
emulation core for the following machines:

* Apple Macintosh Plus
* IBM PC Model 5150 and 5160
* Atari 1040ST
* Regnecentralen RC759 Piccoline

This code is derived from James Friend's [PCE.js](https://github.com/jsdf/pce) port of PCE, which in turn is based off
Hampa Hug's [PCE](http://www.hampa.ch/pce) which is written in ANSI C.

## How does this code differ from Hampa Hug's PCE distribution?

1. The main loop is modified so that it can be run by Emscripten.
2. Exposed to JavaScript the ability to send messages to the emulator via `*_set_msg` to allow for on-the-fly disk insertion.
3. Exposed some of the e8530.c routines to JavaScript to support experiments with LocalTalk emulation

## How does this code differ from James Friend's PCE.js?

1. James' build scripts and UI have been removed and replaced with the code at [retroweb-vintage-computer-museum](https://github.com/marciot/retroweb-vintage-computer-museum).
2. Build instructions have been revised for the Emscripten available as of August 7, 2016 (emcc version 1.36.0)
3. Upstream changes from Hampa's repository have been pulled as of August 7, 2016

## Build instructions

Make sure you have a working version of Emscripten. Do this by following the [Emscripten tutorial](https://kripken.github.io/emscripten-site/index.html).

Make sure your Emscripten toolchain is up to date by executing the following commands:

```
./emsdk update
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

Use the following commands to clone the emulator code and build the emulators:

```
git clone https://github.com/marciot/retroweb-pcejs-jsdf.git
cd retroweb-pcejs-jsdf

emconfigure ./configure    \
   --disable-cpm80         \
   --disable-sims32        \
   --disable-simarm        \
   --disable-sim405        \
   --disable-sim6502       \
   --disable-tun           \
   --disable-char-ppp      \
   --disable-char-pty      \
   --disable-char-slip     \
   --disable-char-tcp      \
   --disable-char-termios  \
   --disable-sound-oss     \
   --disable-readline      \
   --enable-char-posix     

OPTIMIZATION_FLAGS="-O3"

export CFLAGS="$OPTIMIZATION_FLAGS"
emmake make clean
emmake make

mv src/arch/macplus/pce-macplus src/arch/pce-macplus.bc
mv src/arch/rc759/pce-rc759     src/arch/pce-rc759.bc
mv src/arch/ibmpc/pce-ibmpc     src/arch/pce-ibmpc.bc
mv src/arch/atarist/pce-atarist src/arch/pce-atarist.bc

EMCC_OPTS="$OPTIMIZATION_FLAGS"

emcc src/arch/pce-macplus.bc $EMCC_OPTS -o pce-macplus.js -s EXPORTED_FUNCTIONS='["_main","_mac_get_sim","_mac_set_msg","_e8530_set_reg","_e8530_get_reg","_e8530_set_rts","_e8530_set_dcd","_e8530_set_cts"]'
emcc src/arch/pce-rc759.bc   $EMCC_OPTS -o pce-rc759.js   -s EXPORTED_FUNCTIONS='["_main","_rc_get_sim","_rc759_set_msg"]'
emcc src/arch/pce-ibmpc.bc   $EMCC_OPTS -o pce-ibmpc.js   -s EXPORTED_FUNCTIONS='["_main","_pc_get_sim","_pc_set_msg"]'
emcc src/arch/pce-atarist.bc $EMCC_OPTS -o pce-atarist.js -s EXPORTED_FUNCTIONS='["_main","_st_get_sim","_st_set_msg"]'

sed pce-macplus.js -i -e 's/function _SDL_CreateRGBSurfaceFrom/function _SDL_CreateRGBSurfaceFrom_disabled/'
sed pce-rc759.js   -i -e 's/function _SDL_CreateRGBSurfaceFrom/function _SDL_CreateRGBSurfaceFrom_disabled/'
sed pce-ibmpc.js   -i -e 's/function _SDL_CreateRGBSurfaceFrom/function _SDL_CreateRGBSurfaceFrom_disabled/'
sed pce-atarist.js -i -e 's/function _SDL_CreateRGBSurfaceFrom/function _SDL_CreateRGBSurfaceFrom_disabled/'
```

The "pce-xxx.js" files can be used as drop in replacements for those in the "emulators/pce-xxx" directories of the
[retroweb-vintage-computer-museum](https://github.com/marciot/retroweb-vintage-computer-museum) distribution.

## How do I pull changes from the upstream repository?

Although James Friend no longer updates his PCE.js port, Hampa Hug continues to do work on the emulator code. I have only tested the code included in this GitHub repository, but if you would like to merge upstream changes yourself, you may do so by issuing the following commands:

```
~/retroweb-pcejs-jsdf$ git remote add upstream git://git.hampa.ch/pce.git
~/retroweb-pcejs-jsdf$ git remote -v
~/retroweb-pcejs-jsdf$ git fetch upstream
~/retroweb-pcejs-jsdf$ git merge upstream/master
```

**Reference:** https://help.github.com/articles/syncing-a-fork

## Why is SDL_CreateRGBSurfaceFrom being disabled?

The `SDL_CreateRGBSurfaceFrom` that ships with Emscripten only supports a depth of 32. James Friend patched this routine to support a
depth of 8 bits, required by PCE, but his repository did so through a modified version of the Emscripten toolchain which wasn't
kept up to date.

Instead of doing that, I rename the function after compiling the JavaScript files (you could also manually delete
the entire function to save space) and then provide the patch elsewhere (namely, in "emulators/emscripten-interface.js" from [retroweb-vintage-computer-museum](https://github.com/marciot/retroweb-vintage-computer-museum)).