MINIGUI_GAME_VERSION = develop
MINIGUI_GAME_SITE = $(TOPDIR)/../external/minigui_game
MINIGUI_GAME_SITE_METHOD = local

MINIGUI_GAME_LICENSE = Apache V2.0
MINIGUI_GAME_LICENSE_FILES = NOTICE
CC="$(TARGET_CC)"
PROJECT_DIR="$(@D)"

MINIGUI_MAKE_ENV=$(TARGET_MAKE_ENV)

ifeq ($(BR2_PACKAGE_FFMPEG_SWSCALE),y)
MINIGUI_MAKE_ENV += ENABLE_VIDEO=1
endif

define MINIGUI_GAME_IMAGE_COPY
        mkdir -p $(TARGET_DIR)/usr/local/share/
        cp -r $(PROJECT_DIR)/minigui $(TARGET_DIR)/usr/local/share/
        cp -r $(PROJECT_DIR)/minigui/MiniGUI.cfg.$(MINIGUI_TARGET) \
			  $(TARGET_DIR)/etc/MiniGUI.cfg
endef

define MINIGUI_GAME_BUILD_CMDS
	$(MINIGUI_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)"
endef

define MINIGUI_GAME_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0644 -D $($(PKG)_PKGDIR)/61-powersupply.rules $(TARGET_DIR)/lib/udev/rules.d/
        $(INSTALL) -D -m 755 $(@D)/game $(TARGET_DIR)/usr/bin/ && $(MINIGUI_GAME_IMAGE_COPY)
endef

$(eval $(generic-package))
