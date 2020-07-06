ifneq ($(TARGET_SIMULATOR),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= bqtool.c
LOCAL_MODULE := bqtool
LOCAL_STATIC_LIBRARIES := libcutils libc
LOCAL_FORCE_STATIC_EXECUTABLE := true
include $(BUILD_EXECUTABLE)

endif  # TARGET_SIMULATOR != true
