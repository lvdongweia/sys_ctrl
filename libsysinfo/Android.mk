LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	RobotSysInfo.cpp

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libbinder \

LOCAL_C_INCLUDES := \
	external/robot-canbus/include \
    $(LOCAL_PATH)/../include \
    $(LOCAL_PATH)/../include/libsysinfo

LOCAL_MODULE_TAGS := eng optional
LOCAL_CFLAGS := -O2 -Wall -g
LOCAL_MODULE:= libsysinfo

include $(BUILD_SHARED_LIBRARY)
