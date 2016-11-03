LOCAL_PATH := $(call my-dir)

#####RM_SYSCTRL

include $(CLEAR_VARS)
LOCAL_SRC_FILES := src/rm_sysctrl.c \
	           src/rm_sysctrl_cli.c \
	           src/rm_sysctrl_service.cpp \
	           src/rm_sysctrl_uevent.c \
	           src/rm_sysctrl_fault.c \
	           src/rm_sysctrl_fault_report.c \
	           src/rm_sysctrl_subsys.c \
	           src/rm_sysctrl_comm.c \
	           src/rm_sysctrl_fsm.c \
	           src/rm_sysctrl_can.c \
	           src/rm_sysctrl_gpio.c \
	           src/rm_sysctrl_vbdev.c \
	           src/rm_sysctrl_conf.c \
	           src/rm_sysctrl_log.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../robot-canbus/include \
	$(LOCAL_PATH)/../robot-common/include \
	$(KERNEL_HEADERS)

LOCAL_SHARED_LIBRARIES := \
                libutils \
                liblog \
                librm_can \
                librobot_config \
                librobot_sn \
		libc \
		libcutils \
		libbinder \
		libhardware


LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE = rm_sysctrl
include $(BUILD_EXECUTABLE)

#####RM_SYSCTRL_CLI_CLIENT

include $(CLEAR_VARS)
LOCAL_SRC_FILES := src/rm_sysctrl_cli_client.c \
	           src/rm_sysctrl_service_client.cpp

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../robot-canbus/include \
	$(LOCAL_PATH)/../robot-common/include \
	$(KERNEL_HEADERS)

LOCAL_SHARED_LIBRARIES := \
                libutils \
                liblog \
                librm_can \
                librobot_config \
                librobot_sn \
		libc \
		libcutils \
		libbinder \
		libhardware

LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE = rsc_cli
include $(BUILD_EXECUTABLE)

include $(LOCAL_PATH)/libsysinfo/Android.mk
