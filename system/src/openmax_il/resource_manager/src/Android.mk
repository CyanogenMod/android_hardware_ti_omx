LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= \
	ResourceManager.c
	
LOCAL_C_INCLUDES += \
	$(TI_OMX_INCLUDES) \
	$(TI_BRIDGE_TOP)/api/inc \
	$(TI_OMX_SYSTEM)/resource_manager/inc \
	$(TI_OMX_SYSTEM)/resource_manager_proxy/inc \
	$(TI_OMX_SYSTEM)/omx_policy_manager/inc \
	$(TI_OMX_SYSTEM)/resource_manager/resource_activity_monitor/inc \
	$(TI_OMX_SYSTEM)/perf/inc

LOCAL_SHARED_LIBRARIES := \
	libdl 	\
	libcutils \
	libbridge \
	libRAM \
	libPERF
	
LOCAL_STATIC_LIBRARIES := \
	libqos


LOCAL_CFLAGS := $(TI_OMX_CFLAGS)

ifeq ($(ENABLE_RMPM_STUB), 1)
    LOCAL_CFLAGS += -D__ENABLE_RMPM_STUB__
endif

LOCAL_MODULE:= OMXResourceManager

include $(BUILD_EXECUTABLE)

