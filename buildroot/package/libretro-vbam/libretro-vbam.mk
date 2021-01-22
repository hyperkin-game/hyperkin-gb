################################################################################
#
# VBAM
#
################################################################################
#LIBRETRO_VBAM_VERSION = 8041717e1c9993085754b646d3c62ef9422ad652
LIBRETRO_VBAM_VERSION = 36b011c72e5e6f7be4cebb24f3b8324ec209aef0
LIBRETRO_VBAM_SITE = $(call github,libretro,vbam-libretro,$(LIBRETRO_VBAM_VERSION))
LIBRETRO_VBAM_SITE = $(TOPDIR)/package/libretro-vbam/src
LIBRETRO_VBAM_SITE_METHOD = local

LIBRETRO_VBAM_CONF_OPTS += -DENABLE_SDL=OFF
LIBRETRO_VBAM_CONF_OPTS += -DENABLE_GTK=OFF
LIBRETRO_VBAM_CONF_OPTS += -DENABLE_OPENGL=OFF
LIBRETRO_VBAM_CONF_OPTS += -DCMAKE_CXX_FLAGS="-fpermissive"

define LIBRETRO_VBAM_BUILD_CMDS
        CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
               LDFLAGS="$(TARGET_LDFLAGS)" \
               $(MAKE) -C $(@D)/src/libretro -f Makefile \
               CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
               RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
               platform="$(LIBRETRO_PLATFORM)"
endef

define LIBRETRO_VBAM_INSTALL_TARGET_CMDS
        $(INSTALL) -D $(@D)/src/libretro/vbam_libretro.so \
                $(TARGET_DIR)/usr/lib/libretro/vbam_libretro.so
endef

$(eval $(generic-package))

