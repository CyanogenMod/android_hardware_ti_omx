LOCAL_PATH:= $(call my-dir)

include $(LOCAL_PATH)/Config.mk
LOCAL_SRC_FILES:= \
	omx_proxy_h264dec.c
LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.H264D
include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/Config.mk
LOCAL_SRC_FILES:= \
	omx_proxy_h264enc.c
LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.H264E
include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/Config.mk
LOCAL_SRC_FILES:= \
	omx_proxy_jpeg_dec.c
LOCAL_MODULE:= libOMX.TI.DUCATI1.IMAGE.JPEGD
include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/Config.mk
LOCAL_SRC_FILES:= \
	omx_proxy_camera.c
LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.CAMERA
include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/Config.mk
LOCAL_SRC_FILES:= \
	omx_proxy_sample.c
LOCAL_MODULE:= libOMX.TI.DUCATI1.MISC.SAMPLE
include $(BUILD_SHARED_LIBRARY)
