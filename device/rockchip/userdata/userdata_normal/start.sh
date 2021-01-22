#!/bin/sh -x

export XDG_CONFIG_HOME=/data
export XDG_RUNTIME_DIR=/data

GAME_PATH=
GAME_LIB=

export LC_ALL='zh_CN.utf8'

case "$1" in
  0)
    GAME_PATH=/oem/fbalpha/kof97.zip
    GAME_LIB=fbalpha2012_libretro.so
    ;;
  1)
    GAME_PATH=/oem/fceumm/001_Super_Mario_Bros.nes
    GAME_LIB=fceumm_libretro.so
    ;;
  2)
    GAME_PATH=/oem/fbalpha/orlegend.zip
    GAME_LIB=fbalpha2012_libretro.so
    ;;
  3)
    GAME_PATH=/oem/fbalpha/kovsh.zip
    GAME_LIB=fbalpha2012_libretro.so
    ;;
  4)
    GAME_PATH=/oem/fceumm/003_Kage.nes
    GAME_LIB=fceumm_libretro.so
    ;;
  5)
    echo "Have not game resources"
    return
    ;;
  *)
    echo "Have not game resources"
    return
    ;;
esac

/usr/bin/retroarch --fullscreen \
    -c /data/retroarch/retroarch.cfg \
    -L /usr/lib/libretro/$GAME_LIB \
    $GAME_PATH
