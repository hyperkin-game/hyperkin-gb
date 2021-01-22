ifeq ($(IS_BUILD_TEST_APP),true)

SUPPORT_DRM_DISPLAY = false
SUPPORT_RK_DISPLAY = true

#
# RockChip Camera HAL 
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES +=\
        testHal.cpp \

LOCAL_SHARED_LIBRARIES:=

ifeq ($(IS_RK_ISP10),true)
LOCAL_CPPFLAGS += -D RK_ISP10
else
LOCAL_CPPFLAGS += -D RK_ISP11
endif

ifeq ($(IS_USE_RK_V4L2_HEAD),true)
LOCAL_CPPFLAGS += -D USE_RK_V4L2_HEAD_FILES
endif

ifeq ($(IS_ANDROID_OS),true)
LOCAL_CPPFLAGS += -DANDROID_OS
LOCAL_C_INCLUDES += external/stlport/stlport bionic/ bionic/libstdc++/include system/core/libion/include/ \
			system/core/include
LOCAL_SHARED_LIBRARIES += libcutils
endif


ifeq ($(IS_NEED_SHARED_PTR),true)
LOCAL_CPPFLAGS += -D ANDROID_SHARED_PTR
endif

ifeq ($(IS_SUPPORT_ION),true)
LOCAL_CPPFLAGS += -DSUPPORT_ION
LOCAL_SHARED_LIBRARIES += libion
endif

#should include ../inlclude after system/core/include
LOCAL_C_INCLUDES += \
        $(LOCAL_PATH)/include\
        $(LOCAL_PATH)/../include\
	$(LOCAL_PATH)/../include/linux \

ifeq ($(SUPPORT_RK_DISPLAY),true)
LOCAL_SRC_FILES += fb.c 
LOCAL_CPPFLAGS += -DUSE_RK_DISPLAY
endif

ifeq ($(SUPPORT_DRM_DISPLAY),true)
#add drm display src files
LOCAL_SRC_FILES +=\
        $(LOCAL_PATH)/drmDsp/dev.c \
        $(LOCAL_PATH)/drmDsp/bo.c \
        $(LOCAL_PATH)/drmDsp/modeset.c \
	drmDsp.c

LOCAL_C_INCLUDES +=/usr/include/libdrm
LOCAL_SHARED_LIBRARIES += libdrm
LOCAL_CPPFLAGS += -DUSE_DRM_DISPLAY
endif

LOCAL_CFLAGS += -Wno-error=unused-function -Wno-array-bounds
LOCAL_CFLAGS += -DLINUX  -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H -DENABLE_ASSERT
LOCAL_CPPFLAGS += -std=c++11
LOCAL_CPPFLAGS +=  -DLINUX  -DENABLE_ASSERT
LOCAL_SHARED_LIBRARIES += \
		libcam_engine_cifisp libcam_ia 

ifeq ($(IS_NEED_LINK_STLPORT),true)
LOCAL_SHARED_LIBRARIES += \
                libstlport 
endif

LOCAL_MODULE:= camHalTest.bin

include $(BUILD_EXECUTABLE)

endif
