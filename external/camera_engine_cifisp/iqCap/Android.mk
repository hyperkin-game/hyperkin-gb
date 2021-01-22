#
# RockChip Camera HAL 
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES +=\
        iqcap.cpp \

LOCAL_SHARED_LIBRARIES:=

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

LOCAL_MODULE:= iqcap

include $(BUILD_EXECUTABLE)
