LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= \
        WmaDecTest.c \

LOCAL_C_INCLUDES := \
        $(TI_OMX_AUDIO)/wma_dec/inc \
        $(TI_OMX_COMP_C_INCLUDES)

LOCAL_SHARED_LIBRARIES := $(TI_OMX_COMP_SHARED_LIBRARIES) \
        libOMX_Core

LOCAL_CFLAGS := $(TI_OMX_CFLAGS) -DOMX_DEBUG

LOCAL_MODULE:= WmaDecTest_common
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
