IS_ANDROID_OS = false
IS_NEED_SHARED_PTR = false
IS_NEED_COMPILE_STLPORT = false
IS_NEED_LINK_STLPORT = false
IS_NEED_COMPILE_TINYXML2 = true
IS_NEED_COMPILE_EXPAT = true
IS_SUPPORT_ION = false
IS_SUPPORT_DMABUF = true
IS_BUILD_TEST_APP = false
IS_BUILD_DUMPSYS = true
IS_CAM_IA10_API = false
IS_RK_ISP10 = true
IS_USE_RK_V4L2_HEAD = false

ifeq ($(ARCH),arm)
BUILD_TARGET = rk3288_rk3399_arm32
CROSS_COMPILE ?= $(shell pwd)/../../prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
else
BUILD_TARGET = rk3288_rk3399_arm64
CROSS_COMPILE ?= $(shell pwd)/../../prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
endif

#BUILD_TARGET = rk3288_rk3399_arm32
#BUILD_TARGET = rk3288_rk3399_arm64
#BUILD_TARGET = rv1108
BUILD_OUPUT_EXTERNAL_LIBS:=$(CURDIR)/../../out/system/lib/
#CROSS_COMPILE ?= $(shell pwd)/../../prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
#CROSS_COMPILE ?= /home/hkj/rk3288_DR935/toolchains/gcc-linaro-5.5.0-2017.10-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
#CROSS_COMPILE ?= /home/hkj/CVR_CameraHal/prebuilts/toolschain/usr/bin/arm-linux-
#CROSS_COMPILE ?= $(CURDIR)/../../../prebuilts/toolschain/usr/bin/arm-linux-
#CROSS_COMPILE ?= /extra/zyc/android6.0/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-
#CROSS_COMPILE ?=
