LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE = minipcie_ctl
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := minipcie_ctl.c

include $(BUILD_EXECUTABLE)

