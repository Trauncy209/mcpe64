LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := RakNet
MY_SOURCES := $(notdir $(wildcard $(LOCAL_PATH)/RaknetSources/*.cpp))
LOCAL_SRC_FILES := $(addprefix RaknetSources/,$(MY_SOURCES))
LOCAL_C_INCLUDES := $(LOCAL_PATH)/RaknetSources
LOCAL_CFLAGS := -Wno-psabi $(LOCAL_CFLAGS)
LOCAL_CPPFLAGS := -std=gnu++11 -Wno-register $(LOCAL_CPPFLAGS)

include $(BUILD_STATIC_LIBRARY)
