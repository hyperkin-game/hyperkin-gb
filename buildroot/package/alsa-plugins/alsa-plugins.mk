#T###############################################################################
#
# alsa-utils
#
################################################################################

ALSA_PLUGINS_VERSION = 1.1.5
ALSA_PLUGINS_SOURCE = alsa-plugins-$(ALSA_PLUGINS_VERSION).tar.bz2
ALSA_PLUGINS_SITE = ftp://ftp.alsa-project.org/pub/plugins
ALSA_PLUGINS_LICENSE = GPLv2
ALSA_PLUGINS_LICENSE_FILES = COPYING
ALSA_PLUGINS_INSTALL_STAGING = YES
ALSA_PLUGINS_DEPENDENCIES = host-pkgconf alsa-lib \
	$(if $(BR2_PACKAGE_NCURSES),ncurses) \
	$(if $(BR2_PACKAGE_LIBSAMPLERATE),libsamplerate)
# Regenerate aclocal.m4 to pick the patched
# version of alsa.m4 from alsa-lib
#ALSA_PLUGINS_AUTORECONF = YES
#ALSA_PLUGINS_GETTEXTIZE = YES

#ALSA_PLUGINS_CONF_OPTS = \
						 --disable-libtool-lock  \
						 --disable-oss           \
						 --disable-mix           \
						 --disable-usbstream     \
						 --disable-arcamav       \
						 --disable-jack          \
						 --disable-pulseaudio    \
						 --disable-samplerate    \
						 --enable-maemo-plugin   


ALSA_PLUGINS_CONF_OPTS = 

$(eval $(autotools-package))
