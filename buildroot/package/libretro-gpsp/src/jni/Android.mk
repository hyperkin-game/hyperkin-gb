LOCAL_PATH := $(call my-dir)

CORE_DIR     := $(LOCAL_PATH)/..

CORE_LDLIBS  :=
CPU_ARCH     :=
HAVE_DYNAREC :=

COREFLAGS := -DINLINE=inline -D__LIBRETRO__ -DFRONTEND_SUPPORTS_RGB565

ifeq ($(TARGET_ARCH),arm)
   COREFLAGS += -DARM_ARCH
   CPU_ARCH := arm
   HAVE_DYNAREC := 1
else ifeq ($(TARGET_ARCH),x86)
   COREFLAGS += -DHAVE_MMAP
   CPU_ARCH := x86_32
   HAVE_DYNAREC := 1
endif

ifeq ($(HAVE_DYNAREC),1)
  COREFLAGS += -DHAVE_DYNAREC
  CORE_LDLIBS += -Wl,--no-warn-shared-textrel
endif

include $(CORE_DIR)/Makefile.common

GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
   COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

include $(CLEAR_VARS)
LOCAL_MODULE    := retro
LOCAL_SRC_FILES := $(SOURCES_C) $(SOURCES_ASM)
LOCAL_CFLAGS    := $(COREFLAGS) $(INCFLAGS)
LOCAL_LDFLAGS   := -Wl,-version-script=$(CORE_DIR)/link.T
LOCAL_LDLIBS    := $(CORE_LDLIBS)
LOCAL_ARM_MODE  := arm
include $(BUILD_SHARED_LIBRARY)
