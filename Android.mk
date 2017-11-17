INNER_SAVED_LOCAL_PATH := $(LOCAL_PATH)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := uniaudio

LOCAL_C_INCLUDES  := \
	${CLIB_PATH} \
	${UNIAUDIO_SRC_PATH}/include \
	${LOGGER_SRC_PATH} \
	${FS_SRC_PATH} \
	${MPG123_SRC_PATH} \
	${OPENAL_SRC_PATH} \
	${MULTITASK_SRC_PATH}/include \
	${MEMMGR_SRC_PATH}/include \

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,,$(shell find $(LOCAL_PATH) -name "*.cpp" -print)) \

LOCAL_CPPFLAGS  := -std=c++14

LOCAL_STATIC_LIBRARIES := \
	logger \
	fs \
	multitask \

include $(BUILD_STATIC_LIBRARY)	

LOCAL_PATH := $(INNER_SAVED_LOCAL_PATH)