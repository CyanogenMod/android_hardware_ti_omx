LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

TI_BRIDGE_TOP := hardware/ti/omap3/dspbridge

LOCAL_SRC_FILES:= \
	LCML_DspCodec.c

LOCAL_C_INCLUDES += \
	$(TI_OMX_INCLUDES) \
	$(TI_BRIDGE_TOP)/api/inc		\
	$(TI_OMX_SYSTEM)/lcml/inc	

LOCAL_SHARED_LIBRARIES := \
	libdl 		\
	libbridge	\
	libOMX_Core


LOCAL_CFLAGS := $(TI_OMX_CFLAGS)

LOCAL_MODULE:= libLCML

include $(BUILD_SHARED_LIBRARY)
