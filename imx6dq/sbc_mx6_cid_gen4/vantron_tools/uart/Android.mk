LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE = tuart
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := tuart.c

include $(BUILD_EXECUTABLE)
