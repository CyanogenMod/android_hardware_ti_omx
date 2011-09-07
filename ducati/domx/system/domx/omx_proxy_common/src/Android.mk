LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= \
	omx_proxy_common.c \
	log_func_calls.c 

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../../ \
	$(LOCAL_PATH)/../../omx_rpc/inc \
	$(LOCAL_PATH)/../../../domx \
	hardware/ti/omx/ducati/domx/system/omx_core/inc \
	hardware/ti/omx/ducati/domx/system/mm_osal/inc \
	hardware/ti/tiler \
	hardware/ti/syslink/syslink/d2c \
	hardware/ti/syslink/syslink/api/include \
	hardware/ti/omx/ducati/domx/system/domx/omx_proxy_common/src/

LOCAL_CFLAGS := -pipe -fomit-frame-pointer -Wall  -Wno-trigraphs -Werror-implicit-function-declaration  -fno-strict-aliasing -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -msoft-float -Uarm -DMODULE -D__LINUX_ARM_ARCH__=7  -fno-common -DLINUX -fpic
LOCAL_CFLAGS += -D_Android

LOCAL_SHARED_LIBRARIES := \
		libOMX_CoreOsal \
		libipcutils \
		libsysmgr \
		libipc \
		librcm \
		libnotify \
		libc \
		liblog	\
		libtimemmgr \
		libd2cmap \
		libomx_rpc

# Uncomment the below 2 lines to enable the run time
# dump of NV12 buffers from Decoder/Camera
# based on setprop control
#LOCAL_CFLAGS += -DENABLE_RAW_BUFFERS_DUMP_UTILITY
#LOCAL_SHARED_LIBRARIES += libcutils

LOCAL_MODULE:= libomx_proxy_common
LOCAL_MODULE_TAGS:= optional

include $(BUILD_SHARED_LIBRARY)
