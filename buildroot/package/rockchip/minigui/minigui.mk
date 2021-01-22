MINIGUI_SITE = $(TOPDIR)/../external/minigui
MINIGUI_VERSION = master
MINIGUI_SITE_METHOD = local
MINIGUI_INSTALL_STAGING = YES

MINIGUI_LICENSE_FILES = COPYING
MINIGUI_LICENSE = GPLv3
MINIGUI_AUTORECONF = YES
MINIGUI_DEPENDENCIES = jpeg libpng freetype

##    --host=arm-linux  
##    --target=arm-linux 
MINIGUI_CONF_OPTS = \
    --build=i386-linux \
    --with-osname=linux \
    --disable-videopcxvfb \
    --with-ttfsupport=none \
    --enable-autoial \
    --disable-vbfsupport \
    --disable-tslibial \
    --disable-textmode \
    --enable-vbfsupport \
    --disable-pcxvfb \
    --disable-dlcustomial \
    --disable-dummyial \
    --disable-jpgsupport \
    --disable-fontcourier \
    --disable-screensaver \
    --enable-RKKeybroad_ial \
    --enable-jpgsupport \
    --disable-fontsserif \
    --disable-fontsystem \
    --disable-flatlf \
    --disable-skinlfi \
    --disable-mousecalibrate \
    --disable-dblclk \
    --disable-consoleps2 \
    --disable-consolems \
    --disable-consolems3 \
    --disable-rbfterminal \
    --disable-rbffixedsys \
    --disable-vbfsupport \
    --disable-splash \
    --enable-videoshadow \
    --disable-static \
    --enable-shared \
    --disable-procs \
    --disable-cursor \
    --with-runmode=ths \
    --disable-incoreres \
    --enable-pngsupport \
    --enable-ttfsupport \
    --with-ttfsupport=ft2 \
    --with-ft2-includes=$(STAGING_DIR)/usr/include/freetype2 \
    --with-pic

ifeq ($(BR2_PACKAGE_LIBDRM),y)
MINIGUI_TARGET=drmcon

MINIGUI_CONF_OPTS += \
    --enable-videodrmcon \
    --disable-videofbcon

MINIGUI_DEPENDENCIES += pixman
else
MINIGUI_TARGET=fbcon

MINIGUI_CONF_OPTS += \
    --enable-videofbcon
endif

MINIGUI_CONF_OPTS += \
    --with-targetname=$(MINIGUI_TARGET)

$(eval $(autotools-package))
