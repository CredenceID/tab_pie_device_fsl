# this is now the default FreeType build for Android
#
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# compile in ARM mode, since the glyph loader/renderer is a hotspot
# when loading complex pages in the browser
#
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES:= \
	tools/i2cdetect.c \
	tools/i2cbusses.c 
LOCAL_MODULE := i2cdetect
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES:= \
	tools/util.c \
	tools/i2cdump.c \
	tools/i2cbusses.c 
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include
LOCAL_MODULE := i2cdump
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES:= \
	tools/util.c \
	tools/i2cbusses.c \
	tools/i2cset.c 
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include
LOCAL_MODULE := i2cset
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES:= \
	tools/util.c \
	tools/i2cbusses.c \
	tools/i2cget.c 
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include
LOCAL_MODULE := i2cget
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

