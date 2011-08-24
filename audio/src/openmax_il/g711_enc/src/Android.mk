LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= \
        OMX_G711Enc_ComponentThread.c                \
        OMX_G711Enc_Utils.c                \
        OMX_G711Encoder.c

LOCAL_C_INCLUDES := $(TI_OMX_COMP_C_INCLUDES) \
        $(TI_OMX_SYSTEM)/common/inc                \
        $(TI_OMX_AUDIO)/g711_enc/inc

LOCAL_SHARED_LIBRARIES := $(TI_OMX_COMP_SHARED_LIBRARIES) \
        libbridge


LOCAL_LDLIBS += \
        -lpthread \
        -ldl \
        -lsdl

LOCAL_CFLAGS := $(TI_OMX_CFLAGS) -DOMAP_2430

LOCAL_MODULE:= libOMX.TI.G711.encode
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
