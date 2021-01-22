################################################################################
#
# MGBA
#
################################################################################
LIBRETRO_MGBA_VERSION = ca9c9119ded9c112eafd7301460ac25c2765731a
LIBRETRO_MGBA_SITE = $(call github,mgba-emu,mgba,$(LIBRETRO_MGBA_VERSION))
LIBRETRO_MGBA_SITE = $(TOPDIR)/package/libretro-mgba/src
LIBRETRO_MGBA_SITE_METHOD = local

# Configs for libretro
LIBRETRO_MGBA_CONF_OPTS += -DBUILD_LIBRETRO=ON
LIBRETRO_MGBA_CONF_OPTS += -DBUILD_QT=OFF
LIBRETRO_MGBA_CONF_OPTS += -DBUILD_SDL=OFF

# Reusing raspi configs for ARM
ifeq ($(BR2_arm),y)
	LIBRETRO_MGBA_CONF_OPTS += -DBUILD_RASPI=ON
endif

ifeq ($(BR2_PACKAGE_HAS_LIBEGL),y)
	LIBRETRO_MGBA_CONF_OPTS += -DBUILD_GLES2=ON
endif

define LIBRETRO_MGBA_BUILD_CMDS
        CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
               LDFLAGS="$(TARGET_LDFLAGS)" \
               $(MAKE) -C $(@D) -f Makefile.libretro \
               CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
               RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
               platform="classic_armv7_a7"
endef

define LIBRETRO_MGBA_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mgba_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/mgba_libretro.so
endef

$(eval $(generic-package))
