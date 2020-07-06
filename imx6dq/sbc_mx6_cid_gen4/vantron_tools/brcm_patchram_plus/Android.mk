LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE = brcm_patchram_plus


LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := brcm_patchram_plus.c

include $(BUILD_EXECUTABLE)
