LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= \
	OMX_VideoDec_Thread.c \
	OMX_VideoDec_Utils.c \
	OMX_VideoDecoder.c

LOCAL_C_INCLUDES += \
	kernel/include/linux \
	$(TI_BRIDGE_TOP)/api/inc \
	$(TI_OMX_SYSTEM)/omx_core/inc \
	$(TI_OMX_SYSTEM)/lcml/inc \
	$(TI_OMX_SYSTEM)/common/inc \
	$(TI_OMX_VIDEO)/video_decode/inc

LOCAL_SHARED_LIBRARIES := \
	libdl \
	libbridge \
	libOMX_Core \
	libLCML	\
	liblog

LOCAL_LDLIBS += \
	-lpthread \


LOCAL_CFLAGS += -Wall -fpic -pipe -DSTATIC_TABLE -O0 -DOMAP_2430

LOCAL_MODULE:= libOMX.TI.Video.Decoder

include $(BUILD_SHARED_LIBRARY)
