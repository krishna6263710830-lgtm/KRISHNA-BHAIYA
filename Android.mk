LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := LocalHacks

LOCAL_CFLAGS := -Wno-error=format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w
LOCAL_CFLAGS += -fno-rtti -fno-exceptions -fpermissive
LOCAL_CPPFLAGS := -Wno-error=format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w -Werror -s -std=c++17
LOCAL_CPPFLAGS += -Wno-error=c++11-narrowing -fms-extensions -fno-rtti -fno-exceptions -fpermissive
LOCAL_LDFLAGS += -Wl,--gc-sections,--strip-all, -llog
LOCAL_ARM_MODE := arm

LOCAL_LDLIBS += -llog

LOCAL_C_INCLUDES := $(LOCAL_PATH)/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/curl
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/curl/curl-android-$(TARGET_ARCH_ABI)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/curl/openssl-android-$(TARGET_ARCH_ABI)/include

SOURCE_MOD += $(wildcard $(LOCAL_PATH)/include/fakedlfcn/*.cpp)
SOURCE_MOD += $(wildcard $(LOCAL_PATH)/include/KittyMemory/*.cpp)
SOURCE_MOD += $(wildcard $(LOCAL_PATH)/xhook/*.c)
SOURCE_MOD += $(wildcard $(LOCAL_PATH)/imgui/*.c*)
SOURCE_MOD += $(wildcard $(LOCAL_PATH)/imgui/backends/*.c*)

LOCAL_SRC_FILES := main.cpp \
    tools.cpp \
    InfoDisplay.cpp \
    

LOCAL_SRC_FILES += $(SOURCE_MOD:$(LOCAL_PATH)/%=%)

LOCAL_CPP_FEATURES := exceptions

LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2 -lGLESv3 -lGLESv1_CM -lz

LOCAL_STATIC_LIBRARIES := libdobby libcurl libssl libcrypto libfoxcheats

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libdobby
LOCAL_SRC_FILES := include/dobby/libs/$(TARGET_ARCH_ABI)/libdobby.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libfoxcheats
LOCAL_SRC_FILES := include/MemoryTool/libs/$(TARGET_ARCH_ABI)/libfoxcheats.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcurl
LOCAL_SRC_FILES := include/curl/curl-android-$(TARGET_ARCH_ABI)/lib/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libssl
LOCAL_SRC_FILES := include/curl/openssl-android-$(TARGET_ARCH_ABI)/lib/libssl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcrypto
LOCAL_SRC_FILES := include/curl/openssl-android-$(TARGET_ARCH_ABI)/lib/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)
