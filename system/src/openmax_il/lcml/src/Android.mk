LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

TI_BRIDGE_TOP := hardware/ti/omap3/dspbridge

LOCAL_SRC_FILES:= \
        LCML_DspCodec.c

LOCAL_C_INCLUDES += \
        $(TI_OMX_INCLUDES) \
        $(TI_BRIDGE_TOP)/api/inc \
        $(TI_OMX_SYSTEM)/common/inc \
        $(TI_OMX_SYSTEM)/lcml/inc \
        $(TI_OMX_SYSTEM)/perf/inc

LOCAL_SHARED_LIBRARIES := \
        libdl \
        liblog \
        libbridge \
        libOMX_Core \


ifeq ($(PERF_INSTRUMENTATION),1)
LOCAL_SHARED_LIBRARIES += \
        libPERF
endif

LOCAL_CFLAGS := $(TI_OMX_CFLAGS)

LOCAL_MODULE:= libLCML
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
