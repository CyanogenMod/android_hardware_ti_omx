LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= \
	omx_proxy_h264enc.c

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../../../../system/domx/ \
	$(LOCAL_PATH)/../../../../system/domx/omx_rpc/inc/ \
	$(LOCAL_PATH)/../../../../system/mm_osal/inc/ \
	$(LOCAL_PATH)/../../../../system/omx_core/inc/ \
	hardware/ti/syslink/syslink/api/include \
	hardware/ti/tiler/memmgr
	
LOCAL_SHARED_LIBRARIES := \
		libOMX_CoreOsal \
		libipcutils \
		libsysmgr \
		libipc \
		librcm \
		libnotify \
		libsysmemmgr \
		libc \
		libOMX_Core \
		liblog	\
		libmemmgr \
		libd2cmap \
		
LOCAL_STATIC_LIBRARIES := \
		libomx_proxy_common \
		libomx_rpc \
		libipc_setup \

		
LOCAL_CFLAGS += -pipe -fomit-frame-pointer -Wall  -Wno-trigraphs -Werror-implicit-function-declaration  -fno-strict-aliasing -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -msoft-float -Uarm -DMODULE -D__LINUX_ARM_ARCH__=7  -fno-common -DLINUX -fpic
LOCAL_CFLAGS += -DSTATIC_TABLE

LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.H264E

include $(BUILD_SHARED_LIBRARY)
