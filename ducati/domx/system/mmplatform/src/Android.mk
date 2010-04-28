LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false 
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
ipc_setup.c \
mmplatform.c \

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../../domx \
	$(LOCAL_PATH)/../../omx_core/inc \
	$(LOCAL_PATH)/../../mm_osal/inc \
    $(LOCAL_PATH)/../../domx/omx_rpc/inc \
	hardware/ti/syslink/syslink/api/include
	
	
LOCAL_CFLAGS += -pipe -fomit-frame-pointer -Wall  -Wno-trigraphs -Werror-implicit-function-declaration  -fno-strict-aliasing -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -msoft-float -Uarm -DMODULE -D__LINUX_ARM_ARCH__=7  -fno-common -DLINUX -fpic
LOCAL_CFLAGS += -D_Android

LOCAL_SHARED_LIBRARIES := \
	libipcutils \
	libsysmgr \

LOCAL_MODULE    := libipc_setup

include $(BUILD_STATIC_LIBRARY)

