#!/bin/sh -x
#FC		fceumm_libretro.so
#GB/GBA/GBC	mgba_libretro.so
#MD		genesisplusgx_libretro.so
#N64		mupen64plus_libretro.so
#PS ONE		pcsx_rearmed_libretro.so
#SFC 		snes9x_libretro.so

export XDG_CONFIG_HOME=/data
export XDG_RUNTIME_DIR=/data

GAME_PATH=
GAME_LIB=

export LC_ALL='zh_CN.utf8'

case "$1" in
  0)
    GAME_PATH= /oem/fbalpha/kof97.zip
    GAME_LIB=fbalpha2012_libretro.so
    ;;
  1)
    GAME_PATH= /oem/fceumm/001_Super_Mario_Bros.nes
    GAME_LIB=fceumm_libretro.so
    ;;
  2)
    GAME_PATH=/oem/game/sfa.zip
    GAME_LIB=fbalpha2012_libretro.so
    ;;
  3)
    GAME_PATH=/oem/game/kovsh.zip
    GAME_LIB=fbalpha2012_libretro.so
    ;;
  4)
    GAME_PATH=/oem/game/003_Kage.nes
    GAME_LIB=fceumm_libretro.so
    ;;
  5)
    GAME_PATH=/oem/game/SONIC 2.smd
    GAME_LIB=genesisplusgx_libretro.so
    ;;
  6)
    GAME_PATH=
    GAME_LIB=snes9x_libretro.so
    ;;
  7)
    GAME_PATH=
    GAME_LIB=mgba_libretro.so
    ;;
  8)
    GAME_PATH=
    GAME_LIB=mupen64plus_libretro.so
    ;;
  9)
    GAME_PATH=
    GAME_LIB=pcsx_rearmed_libretro.so
    ;;
  *)
    echo "Have not game resources"
    return
    ;;
esac

/usr/bin/retroarch \
    -c "$2"\
    -L /usr/lib/libretro/$GAME_LIB "$3"\
