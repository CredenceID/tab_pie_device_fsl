# this is now the default FreeType build for Android
#
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES:= mxc_i2c_slave_test.c
LOCAL_MODULE := mxc_i2c_slave_test
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)
