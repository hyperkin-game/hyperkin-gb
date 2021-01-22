################################################################################
#
# FBALPHA2012
#
################################################################################
LIBRETRO_FBALPHA2012_VERSION = c1de0e8c43d6560d9232e57ada6bbb8460168b5a
LIBRETRO_FBALPHA2012_SITE = $(call github,libretro,fbalpha2012,$(LIBRETRO_FBALPHA2012_VERSION))

LIBRETRO_FBALPHA2012_SUBDIR = svn-current/trunk

define LIBRETRO_FBALPHA2012_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS) -lstdc++ -lm" \
	       $(MAKE) -C $(LIBRETRO_FBALPHA2012_SRCDIR) -f makefile.libretro \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)"
endef

define LIBRETRO_FBALPHA2012_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(LIBRETRO_FBALPHA2012_SRCDIR)/fbalpha2012_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/fbalpha2012_libretro.so
endef

$(eval $(generic-package))
