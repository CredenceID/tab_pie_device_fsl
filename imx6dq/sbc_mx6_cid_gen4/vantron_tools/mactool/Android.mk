LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#Lamson.Luo:spec for define 
ifeq ($(SPEC_FOR_5071_TMFP),true)
  LOCAL_CFLAGS += -DCONFIG_SPEC_FOR_5071_TMFP
else
endif

LOCAL_MODULE = mactool
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := mactool.c

include $(BUILD_EXECUTABLE)

