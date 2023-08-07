ARCHS=armv7 armv7s arm64 arm64e
TARGET = iphone:clang:latest:12.0

include $(THEOS)/makefiles/common.mk

TOOL_NAME = jetsamctl
jetsamctl_CFLAGS = -Wall -Wpedantic -Wextra -Wunused-variable -Wobjc-method-access -Werror -Wformat -Wno-incompatible-pointer-types-discards-qualifiers
jetsamctl_FILES = main.c
jetsamctl_CODESIGN_FLAGS = -Sentitlements.xml
include $(THEOS_MAKE_PATH)/tool.mk

before-all::
	@if [ ! -f "$(THEOS_INCLUDE_PATH)/sys/kern_memorystatus.h" ]; then \
		mkdir -p "$(THEOS_INCLUDE_PATH)/sys"; \
		curl -s -o "$(THEOS_INCLUDE_PATH)/sys/kern_memorystatus.h" -L "http://www.opensource.apple.com/source/xnu/xnu-7195.141.2/bsd/sys/kern_memorystatus.h?txt"; \
	fi
