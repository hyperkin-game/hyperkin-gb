################################################################################
#
# ffmpeg
#
################################################################################

FFMPEG3_VERSION = 3.2.7
FFMPEG3_SOURCE = ffmpeg-$(FFMPEG3_VERSION).tar.xz
FFMPEG3_SITE = http://ffmpeg.org/releases
FFMPEG3_INSTALL_STAGING = YES

FFMPEG3_LICENSE = LGPLv2.1+, libjpeg license
FFMPEG3_LICENSE_FILES = LICENSE.md COPYING.LGPLv2.1
ifeq ($(BR2_PACKAGE_FFMPEG_GPL),y)
FFMPEG3_LICENSE += and GPLv2+
FFMPEG3_LICENSE_FILES += COPYING.GPLv2
endif

FFMPEG3_CONF_OPTS = \
	--prefix=/usr \
	--enable-avfilter \
	--disable-version3 \
	--enable-logging \
	--enable-optimizations \
	--disable-extra-warnings \
	--enable-avdevice \
	--enable-avcodec \
	--enable-avformat \
	--disable-x11grab \
	--enable-network \
	--disable-gray \
	--enable-swscale-alpha \
	--disable-small \
	--enable-dct \
	--enable-fft \
	--enable-mdct \
	--enable-rdft \
	--disable-crystalhd \
	--disable-dxva2 \
	--enable-runtime-cpudetect \
	--disable-hardcoded-tables \
	--disable-memalign-hack \
	--disable-mipsdspr2 \
	--disable-msa \
	--enable-hwaccels \
	--disable-avisynth \
	--disable-frei0r \
	--disable-libopencore-amrnb \
	--disable-libopencore-amrwb \
	--disable-libopencv \
	--disable-libcdio \
	--disable-libdc1394 \
	--disable-libgsm \
	--disable-libilbc \
	--disable-libnut \
	--disable-libopenjpeg \
	--disable-libschroedinger \
	--disable-libvo-amrwbenc \
	--disable-symver \
	--disable-doc

FFMPEG3_DEPENDENCIES += $(if $(BR2_PACKAGE_LIBICONV),libiconv) host-pkgconf

ifeq ($(BR2_PACKAGE_FFMPEG_GPL),y)
FFMPEG3_CONF_OPTS += --enable-gpl
else
FFMPEG3_CONF_OPTS += --disable-gpl
endif

ifeq ($(BR2_PACKAGE_FFMPEG_NONFREE),y)
FFMPEG3_CONF_OPTS += --enable-nonfree
else
FFMPEG3_CONF_OPTS += --disable-nonfree
endif

ifeq ($(BR2_PACKAGE_FFMPEG_FFMPEG),y)
FFMPEG3_CONF_OPTS += --enable-ffmpeg
else
FFMPEG3_CONF_OPTS += --disable-ffmpeg
endif

ifeq ($(BR2_PACKAGE_FFMPEG_FFPLAY),y)
FFMPEG3_DEPENDENCIES += sdl
FFMPEG3_CONF_OPTS += --enable-ffplay
FFMPEG3_CONF_ENV += SDL_CONFIG=$(STAGING_DIR)/usr/bin/sdl-config
else
FFMPEG3_CONF_OPTS += --disable-ffplay
endif

ifeq ($(BR2_PACKAGE_FFMPEG_FFSERVER),y)
FFMPEG3_CONF_OPTS += --enable-ffserver
else
FFMPEG3_CONF_OPTS += --disable-ffserver
endif

ifeq ($(BR2_PACKAGE_FFMPEG_AVRESAMPLE),y)
FFMPEG3_CONF_OPTS += --enable-avresample
else
FFMPEG3_CONF_OPTS += --disable-avresample
endif

ifeq ($(BR2_PACKAGE_FFMPEG_FFPROBE),y)
FFMPEG3_CONF_OPTS += --enable-ffprobe
else
FFMPEG3_CONF_OPTS += --disable-ffprobe
endif

ifeq ($(BR2_PACKAGE_FFMPEG_POSTPROC),y)
FFMPEG3_CONF_OPTS += --enable-postproc
else
FFMPEG3_CONF_OPTS += --disable-postproc
endif

ifeq ($(BR2_PACKAGE_FFMPEG_SWSCALE),y)
FFMPEG3_CONF_OPTS += --enable-swscale
else
FFMPEG3_CONF_OPTS += --disable-swscale
endif

ifneq ($(call qstrip,$(BR2_PACKAGE_FFMPEG_ENCODERS)),all)
FFMPEG3_CONF_OPTS += --disable-encoders \
	$(foreach x,$(call qstrip,$(BR2_PACKAGE_FFMPEG_ENCODERS)),--enable-encoder=$(x))
endif

ifneq ($(call qstrip,$(BR2_PACKAGE_FFMPEG_DECODERS)),all)
FFMPEG3_CONF_OPTS += --disable-decoders \
	$(foreach x,$(call qstrip,$(BR2_PACKAGE_FFMPEG_DECODERS)),--enable-decoder=$(x))
endif

ifneq ($(call qstrip,$(BR2_PACKAGE_FFMPEG_MUXERS)),all)
FFMPEG3_CONF_OPTS += --disable-muxers \
	$(foreach x,$(call qstrip,$(BR2_PACKAGE_FFMPEG_MUXERS)),--enable-muxer=$(x))
endif

ifneq ($(call qstrip,$(BR2_PACKAGE_FFMPEG_DEMUXERS)),all)
FFMPEG3_CONF_OPTS += --disable-demuxers \
	$(foreach x,$(call qstrip,$(BR2_PACKAGE_FFMPEG_DEMUXERS)),--enable-demuxer=$(x))
endif

ifneq ($(call qstrip,$(BR2_PACKAGE_FFMPEG_PARSERS)),all)
FFMPEG3_CONF_OPTS += --disable-parsers \
	$(foreach x,$(call qstrip,$(BR2_PACKAGE_FFMPEG_PARSERS)),--enable-parser=$(x))
endif

ifneq ($(call qstrip,$(BR2_PACKAGE_FFMPEG_BSFS)),all)
FFMPEG3_CONF_OPTS += --disable-bsfs \
	$(foreach x,$(call qstrip,$(BR2_PACKAGE_FFMPEG_BSFS)),--enable-bsfs=$(x))
endif

ifneq ($(call qstrip,$(BR2_PACKAGE_FFMPEG_PROTOCOLS)),all)
FFMPEG3_CONF_OPTS += --disable-protocols \
	$(foreach x,$(call qstrip,$(BR2_PACKAGE_FFMPEG_PROTOCOLS)),--enable-protocol=$(x))
endif

ifneq ($(call qstrip,$(BR2_PACKAGE_FFMPEG_FILTERS)),all)
FFMPEG3_CONF_OPTS += --disable-filters \
	$(foreach x,$(call qstrip,$(BR2_PACKAGE_FFMPEG_FILTERS)),--enable-filter=$(x))
endif

ifeq ($(BR2_PACKAGE_FFMPEG_INDEVS),y)
FFMPEG3_CONF_OPTS += --enable-indevs
else
FFMPEG3_CONF_OPTS += --disable-indevs
endif

ifeq ($(BR2_PACKAGE_FFMPEG_OUTDEVS),y)
FFMPEG3_CONF_OPTS += --enable-outdevs
else
FFMPEG3_CONF_OPTS += --disable-outdevs
endif

ifeq ($(BR2_TOOLCHAIN_HAS_THREADS),y)
FFMPEG3_CONF_OPTS += --enable-pthreads
else
FFMPEG3_CONF_OPTS += --disable-pthreads
endif

ifeq ($(BR2_PACKAGE_ZLIB),y)
FFMPEG3_CONF_OPTS += --enable-zlib
FFMPEG3_DEPENDENCIES += zlib
else
FFMPEG3_CONF_OPTS += --disable-zlib
endif

ifeq ($(BR2_PACKAGE_BZIP2),y)
FFMPEG3_CONF_OPTS += --enable-bzlib
FFMPEG3_DEPENDENCIES += bzip2
else
FFMPEG3_CONF_OPTS += --disable-bzlib
endif

ifeq ($(BR2_PACKAGE_FDK_AAC)$(BR2_PACKAGE_FFMPEG_NONFREE),yy)
FFMPEG3_CONF_OPTS += --enable-libfdk-aac
FFMPEG3_DEPENDENCIES += fdk-aac
else
FFMPEG3_CONF_OPTS += --disable-libfdk-aac
endif

ifeq ($(BR2_PACKAGE_GNUTLS),y)
FFMPEG3_CONF_OPTS += --enable-gnutls --disable-openssl
FFMPEG3_DEPENDENCIES += gnutls
else
FFMPEG3_CONF_OPTS += --disable-gnutls
ifeq ($(BR2_PACKAGE_OPENSSL),y)
# openssl isn't license compatible with GPL
ifeq ($(BR2_PACKAGE_FFMPEG_GPL)x$(BR2_PACKAGE_FFMPEG_NONFREE),yx)
FFMPEG3_CONF_OPTS += --disable-openssl
else
FFMPEG3_CONF_OPTS += --enable-openssl
FFMPEG3_DEPENDENCIES += openssl
endif
else
FFMPEG3_CONF_OPTS += --disable-openssl
endif
endif

#ifeq ($(BR2_PACKAGE_LIBDCADEC),y)
#FFMPEG3_CONF_OPTS += --enable-libdcadec
#FFMPEG3_DEPENDENCIES += libdcadec
#else
#FFMPEG3_CONF_OPTS += --disable-libdcadec
#endif

ifeq ($(BR2_PACKAGE_FFMPEG_GPL)$(BR2_PACKAGE_LIBEBUR128),yy)
FFMPEG3_DEPENDENCIES += libebur128
endif

ifeq ($(BR2_PACKAGE_LIBOPENH264),y)
FFMPEG3_CONF_OPTS += --enable-libopenh264
FFMPEG3_DEPENDENCIES += libopenh264
else
FFMPEG3_CONF_OPTS += --disable-libopenh264
endif

ifeq ($(BR2_PACKAGE_LIBVORBIS),y)
FFMPEG3_DEPENDENCIES += libvorbis
FFMPEG3_CONF_OPTS += \
	--enable-libvorbis \
	--enable-muxer=ogg \
	--enable-encoder=libvorbis
endif

ifeq ($(BR2_PACKAGE_LIBVA),y)
FFMPEG3_CONF_OPTS += --enable-vaapi
FFMPEG3_DEPENDENCIES += libva
else
FFMPEG3_CONF_OPTS += --disable-vaapi
endif

ifeq ($(BR2_PACKAGE_LIBVDPAU),y)
FFMPEG3_CONF_OPTS += --enable-vdpau
FFMPEG3_DEPENDENCIES += libvdpau
else
FFMPEG3_CONF_OPTS += --disable-vdpau
endif

ifeq ($(BR2_PACKAGE_OPUS),y)
FFMPEG3_CONF_OPTS += --enable-libopus
FFMPEG3_DEPENDENCIES += opus
else
FFMPEG3_CONF_OPTS += --disable-libopus
endif

ifeq ($(BR2_PACKAGE_LIBVPX),y)
FFMPEG3_CONF_OPTS += --enable-libvpx
FFMPEG3_DEPENDENCIES += libvpx
else
FFMPEG3_CONF_OPTS += --disable-libvpx
endif

ifeq ($(BR2_PACKAGE_LIBASS),y)
FFMPEG3_CONF_OPTS += --enable-libass
FFMPEG3_DEPENDENCIES += libass
else
FFMPEG3_CONF_OPTS += --disable-libass
endif

ifeq ($(BR2_PACKAGE_LIBBLURAY),y)
FFMPEG3_CONF_OPTS += --enable-libbluray
FFMPEG3_DEPENDENCIES += libbluray
else
FFMPEG3_CONF_OPTS += --disable-libbluray
endif

ifeq ($(BR2_PACKAGE_RTMPDUMP),y)
FFMPEG3_CONF_OPTS += --enable-librtmp
FFMPEG3_DEPENDENCIES += rtmpdump
else
FFMPEG3_CONF_OPTS += --disable-librtmp
endif

ifeq ($(BR2_PACKAGE_LAME),y)
FFMPEG3_CONF_OPTS += --enable-libmp3lame
FFMPEG3_DEPENDENCIES += lame
else
FFMPEG3_CONF_OPTS += --disable-libmp3lame
endif

ifeq ($(BR2_PACKAGE_LIBMODPLUG),y)
FFMPEG3_CONF_OPTS += --enable-libmodplug
FFMPEG3_DEPENDENCIES += libmodplug
else
FFMPEG3_CONF_OPTS += --disable-libmodplug
endif

ifeq ($(BR2_PACKAGE_SPEEX),y)
FFMPEG3_CONF_OPTS += --enable-libspeex
FFMPEG3_DEPENDENCIES += speex
else
FFMPEG3_CONF_OPTS += --disable-libspeex
endif

ifeq ($(BR2_PACKAGE_LIBTHEORA),y)
FFMPEG3_CONF_OPTS += --enable-libtheora
FFMPEG3_DEPENDENCIES += libtheora
else
FFMPEG3_CONF_OPTS += --disable-libtheora
endif

ifeq ($(BR2_PACKAGE_WAVPACK),y)
FFMPEG3_CONF_OPTS += --enable-libwavpack
FFMPEG3_DEPENDENCIES += wavpack
else
FFMPEG3_CONF_OPTS += --disable-libwavpack
endif

# ffmpeg freetype support require fenv.h which is only
# available/working on glibc.
# The microblaze variant doesn't provide the needed exceptions
ifeq ($(BR2_PACKAGE_FREETYPE)$(BR2_TOOLCHAIN_USES_GLIBC)x$(BR2_microblaze),yyx)
FFMPEG3_CONF_OPTS += --enable-libfreetype
FFMPEG3_DEPENDENCIES += freetype
else
FFMPEG3_CONF_OPTS += --disable-libfreetype
endif

ifeq ($(BR2_PACKAGE_FONTCONFIG),y)
FFMPEG3_CONF_OPTS += --enable-fontconfig
FFMPEG3_DEPENDENCIES += fontconfig
else
FFMPEG3_CONF_OPTS += --disable-fontconfig
endif

ifeq ($(BR2_PACKAGE_X264)$(BR2_PACKAGE_FFMPEG_GPL),yy)
FFMPEG3_CONF_OPTS += --enable-libx264
FFMPEG3_DEPENDENCIES += x264
else
FFMPEG3_CONF_OPTS += --disable-libx264
endif

ifeq ($(BR2_PACKAGE_X265)$(BR2_PACKAGE_FFMPEG_GPL),yy)
FFMPEG3_CONF_OPTS += --enable-libx265
FFMPEG3_DEPENDENCIES += x265
else
FFMPEG3_CONF_OPTS += --disable-libx265
endif

ifeq ($(BR2_X86_CPU_HAS_MMX),y)
FFMPEG3_CONF_OPTS += --enable-yasm
FFMPEG3_DEPENDENCIES += host-yasm
else
ifeq ($(BR2_x86_i586),y)
# Needed to work around a bug with gcc 5.x:
# error: 'asm' operand has impossible constraints
FFMPEG3_CONF_OPTS += --disable-inline-asm
endif
FFMPEG3_CONF_OPTS += --disable-yasm
FFMPEG3_CONF_OPTS += --disable-mmx
endif

ifeq ($(BR2_X86_CPU_HAS_SSE),y)
FFMPEG3_CONF_OPTS += --enable-sse
else
FFMPEG3_CONF_OPTS += --disable-sse
endif

ifeq ($(BR2_X86_CPU_HAS_SSE2),y)
FFMPEG3_CONF_OPTS += --enable-sse2
else
FFMPEG3_CONF_OPTS += --disable-sse2
endif

ifeq ($(BR2_X86_CPU_HAS_SSE3),y)
FFMPEG3_CONF_OPTS += --enable-sse3
else
FFMPEG3_CONF_OPTS += --disable-sse3
endif

ifeq ($(BR2_X86_CPU_HAS_SSSE3),y)
FFMPEG3_CONF_OPTS += --enable-ssse3
else
FFMPEG3_CONF_OPTS += --disable-ssse3
endif

ifeq ($(BR2_X86_CPU_HAS_SSE4),y)
FFMPEG3_CONF_OPTS += --enable-sse4
else
FFMPEG3_CONF_OPTS += --disable-sse4
endif

ifeq ($(BR2_X86_CPU_HAS_SSE42),y)
FFMPEG3_CONF_OPTS += --enable-sse42
else
FFMPEG3_CONF_OPTS += --disable-sse42
endif

ifeq ($(BR2_X86_CPU_HAS_AVX),y)
FFMPEG3_CONF_OPTS += --enable-avx
else
FFMPEG3_CONF_OPTS += --disable-avx
endif

ifeq ($(BR2_X86_CPU_HAS_AVX2),y)
FFMPEG3_CONF_OPTS += --enable-avx2
else
FFMPEG3_CONF_OPTS += --disable-avx2
endif

# Explicitly disable everything that doesn't match for ARM
# FFMPEG "autodetects" by compiling an extended instruction via AS
# This works on compilers that aren't built for generic by default
ifeq ($(BR2_ARM_CPU_ARMV4),y)
FFMPEG3_CONF_OPTS += --disable-armv5te
endif
ifeq ($(BR2_ARM_CPU_ARMV6)$(BR2_ARM_CPU_ARMV7A),y)
FFMPEG3_CONF_OPTS += --enable-armv6
else
FFMPEG3_CONF_OPTS += --disable-armv6 --disable-armv6t2
endif
ifeq ($(BR2_ARM_CPU_HAS_VFPV2),y)
FFMPEG3_CONF_OPTS += --enable-vfp
else
FFMPEG3_CONF_OPTS += --disable-vfp
endif
ifeq ($(BR2_ARM_CPU_HAS_NEON),y)
FFMPEG3_CONF_OPTS += --enable-neon
else
FFMPEG3_CONF_OPTS += --disable-neon
endif

ifeq ($(BR2_mips)$(BR2_mipsel)$(BR2_mips64)$(BR2_mips64el),y)
ifeq ($(BR2_MIPS_SOFT_FLOAT),y)
FFMPEG3_CONF_OPTS += --disable-mipsfpu
else
FFMPEG3_CONF_OPTS += --enable-mipsfpu
endif

ifeq ($(BR2_mips_32r2),y)
FFMPEG3_CONF_OPTS += \
	--enable-mips32r2
else
FFMPEG3_CONF_OPTS += \
	--disable-mips32r2
endif
endif # MIPS

ifeq ($(BR2_POWERPC_CPU_HAS_ALTIVEC),y)
FFMPEG3_CONF_OPTS += --enable-altivec
else
FFMPEG3_CONF_OPTS += --disable-altivec
endif

ifeq ($(BR2_STATIC_LIBS),)
FFMPEG3_CONF_OPTS += --enable-pic
else
FFMPEG3_CONF_OPTS += --disable-pic
endif

ifneq ($(call qstrip,$(BR2_GCC_TARGET_CPU)),)
FFMPEG3_CONF_OPTS += --cpu=$(BR2_GCC_TARGET_CPU)
else ifneq ($(call qstrip,$(BR2_GCC_TARGET_ARCH)),)
FFMPEG3_CONF_OPTS += --cpu=$(BR2_GCC_TARGET_ARCH)
endif

FFMPEG3_CONF_OPTS += $(call qstrip,$(BR2_PACKAGE_FFMPEG_EXTRACONF))

# Override FFMPEG_CONFIGURE_CMDS: FFmpeg does not support --target and others
define FFMPEG3_CONFIGURE_CMDS
	(cd $(FFMPEG3_SRCDIR) && rm -rf config.cache && \
	$(TARGET_CONFIGURE_OPTS) \
	$(TARGET_CONFIGURE_ARGS) \
	$(FFMPEG3_CONF_ENV) \
	./configure \
		--enable-cross-compile \
		--cross-prefix=$(TARGET_CROSS) \
		--sysroot=$(STAGING_DIR) \
		--host-cc="$(HOSTCC)" \
		--arch=$(BR2_ARCH) \
		--target-os="linux" \
		--disable-stripping \
		--pkg-config="$(PKG_CONFIG_HOST_BINARY)" \
		$(SHARED_STATIC_LIBS_OPTS) \
		$(FFMPEG3_CONF_OPTS) \
	)
endef

$(eval $(autotools-package))
