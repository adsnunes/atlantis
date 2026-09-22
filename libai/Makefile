MIN_MACOS_VERSION = 26.0


ifdef CROSS_COMPILE
    ifeq ($(CROSS_COMPILE),x86_64)
        ARCH = x86_64
        ARCH_FLAGS = -target x86_64-apple-macos$(MIN_MACOS_VERSION)
        SWIFT_ARCH_FLAGS = -target x86_64-apple-macos$(MIN_MACOS_VERSION)
    else ifeq ($(CROSS_COMPILE),arm64e)
        ARCH = arm64e
        ARCH_FLAGS = -target arm64e-apple-macos$(MIN_MACOS_VERSION)
        SWIFT_ARCH_FLAGS = -target arm64e-apple-macos$(MIN_MACOS_VERSION)
    else
        $(error Unsupported CROSS_COMPILE target: $(CROSS_COMPILE). Supported: x86_64, arm64e)
    endif
else
    ARCH := $(shell uname -m)
    ARCH_FLAGS = -target $(ARCH)-apple-macos$(MIN_MACOS_VERSION) -march=native -mtune=native
    SWIFT_ARCH_FLAGS = -target $(ARCH)-apple-macos$(MIN_MACOS_VERSION)
endif

BUILD_DIR = build

GIT_TAG := $(shell git describe --tags --exact-match 2>/dev/null || echo "")
ifneq ($(GIT_TAG),)
    VERSION := $(shell echo $(GIT_TAG) | sed 's/^v//')
    MAJOR_VERSION := $(shell echo $(VERSION) | cut -d. -f1)
    MINOR_VERSION := $(shell echo $(VERSION) | cut -d. -f2)
    PATCH_VERSION := $(shell echo $(VERSION) | cut -d. -f3)
else
    MAJOR_VERSION = 0
    MINOR_VERSION = 0
    PATCH_VERSION = 0
    VERSION = $(MAJOR_VERSION).$(MINOR_VERSION).$(PATCH_VERSION)-dev
endif
COMPATIBILITY_VERSION = $(MAJOR_VERSION).0.0

VERSION_DEFINES = -DAI_VERSION_MAJOR=$(MAJOR_VERSION) -DAI_VERSION_MINOR=$(MINOR_VERSION) -DAI_VERSION_PATCH=$(PATCH_VERSION) -DAI_VERSION_STRING=\"$(VERSION)\"

CC = clang
SWIFTC = swiftc

# Base flags
BASE_CFLAGS = -std=c23 -Wall -pthread $(ARCH_FLAGS)

# Release flags
REL_CFLAGS = $(BASE_CFLAGS) -O3 -flto -funroll-loops -fomit-frame-pointer -DNDEBUG
REL_SWIFT_FLAGS = -Ounchecked -whole-module-optimization -parse-as-library \
                  -module-name AIBridge $(SWIFT_ARCH_FLAGS) \
                  -enable-library-evolution -swift-version 5

# Debug flags
DBG_CFLAGS = $(BASE_CFLAGS) -O0 -g -DDEBUG -fno-omit-frame-pointer
DBG_SWIFT_FLAGS = -Onone -g -parse-as-library \
                  -module-name AIBridge $(SWIFT_ARCH_FLAGS) \
                  -enable-library-evolution -swift-version 5

# Object file paths
STATIC_REL_OBJ_DIR = $(BUILD_DIR)/obj/static/$(ARCH)/release
STATIC_DBG_OBJ_DIR = $(BUILD_DIR)/obj/static/$(ARCH)/debug

.PHONY: all clean static-rel static-dbg print-version

all: static-rel

# Static release object files
$(STATIC_REL_OBJ_DIR)/ai.o: ai.c | $(STATIC_REL_OBJ_DIR)
	$(CC) $(REL_CFLAGS) $(VERSION_DEFINES) -c $< -o $@

$(STATIC_REL_OBJ_DIR)/AIBridge.o: bridge.swift | $(STATIC_REL_OBJ_DIR)
	$(SWIFTC) $(REL_SWIFT_FLAGS) -emit-object -static -o $@ $<

# Static debug object files
$(STATIC_DBG_OBJ_DIR)/ai.o: ai.c | $(STATIC_DBG_OBJ_DIR)
	$(CC) $(DBG_CFLAGS) $(VERSION_DEFINES) -c $< -o $@

$(STATIC_DBG_OBJ_DIR)/AIBridge.o: bridge.swift | $(STATIC_DBG_OBJ_DIR)
	$(SWIFTC) $(DBG_SWIFT_FLAGS) -emit-object -static -o $@ $<

# Static release build
static-rel: $(BUILD_DIR)/static/$(ARCH)/release/libai.a

$(BUILD_DIR)/static/$(ARCH)/release/libai.a: $(STATIC_REL_OBJ_DIR)/ai.o $(STATIC_REL_OBJ_DIR)/AIBridge.o | $(BUILD_DIR)/static/$(ARCH)/release
	libtool -static -o $@ $^

# Static debug build
static-dbg: $(BUILD_DIR)/static/$(ARCH)/debug/libai.a

$(BUILD_DIR)/static/$(ARCH)/debug/libai.a: $(STATIC_DBG_OBJ_DIR)/ai.o $(STATIC_DBG_OBJ_DIR)/AIBridge.o | $(BUILD_DIR)/static/$(ARCH)/debug
	libtool -static -o $@ $^

# Directory creation
$(STATIC_REL_OBJ_DIR):
	@mkdir -p $@

$(STATIC_DBG_OBJ_DIR):
	@mkdir -p $@

$(BUILD_DIR)/static/$(ARCH)/release:
	@mkdir -p $@

$(BUILD_DIR)/static/$(ARCH)/debug:
	@mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

print-version:
	@echo $(VERSION)
