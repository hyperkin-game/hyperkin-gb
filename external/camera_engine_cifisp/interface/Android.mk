#
# RockChip Camera HAL 
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS += -Wno-error=unused-function -Wno-array-bounds
LOCAL_CFLAGS += -DLINUX  -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H -DENABLE_ASSERT
LOCAL_CPPFLAGS += -std=c++11
LOCAL_CPPFLAGS += -D_GLIBCXX_USE_C99=1 -DLINUX  -DENABLE_ASSERT

ifeq ($(IS_ANDROID_OS),true)
LOCAL_CPPFLAGS += -DANDROID_OS
LOCAL_C_INCLUDES += external/stlport/stlport bionic/ bionic/libstdc++/include system/core/libion/include/ \
            system/core/include

LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
else
LOCAL_SHARED_LIBRARIES += libpthread
endif

LOCAL_SHARED_LIBRARIES += libcam_ia 

ifeq ($(IS_NEED_SHARED_PTR),true)
LOCAL_CPPFLAGS += -D ANDROID_SHARED_PTR
endif

ifeq ($(IS_RK_ISP10),true)
LOCAL_CPPFLAGS += -D RK_ISP10
else
LOCAL_CPPFLAGS += -D RK_ISP11
endif

ifeq ($(IS_USE_RK_V4L2_HEAD),true)
LOCAL_CPPFLAGS += -D USE_RK_V4L2_HEAD_FILES
endif

ifeq ($(IS_SUPPORT_ION),true)
LOCAL_SHARED_LIBRARIES += libion
endif

ifeq ($(IS_SUPPORT_DMABUF),true)
LOCAL_SHARED_LIBRARIES += libdrm
endif

ifeq ($(IS_NEED_LINK_STLPORT),true)
LOCAL_SHARED_LIBRARIES += libstlport
endif

LOCAL_SRC_FILES += \
	source/rkisp_interface.cpp \
	source/isp_ctrl.cpp \
	source/intel_ia_ctrl.cpp \
	source/arc_rkisp_interface.cpp \
	source/arc_rkisp_adapter.cpp

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
    $(LOCAL_PATH)/include_private \
	$(LOCAL_PATH)/../include \
    $(LOCAL_PATH)/../include/linux \
    $(LOCAL_PATH)/../include/ia

LOCAL_STATIC_LIBRARIES := \
	libcam_hal

LOCAL_MODULE := librkisp
include $(BUILD_SHARED_LIBRARY)
