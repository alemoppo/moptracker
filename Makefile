# MopTracker Makefile
#
# Targets:
#   make          - Build to build/
#   make run      - Build and run
#   make clean    - Remove build/ directory

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
LDFLAGS  ?=

CURDIR   := $(CURDIR)
VENDOR   ?= $(CURDIR)/vendor
BUILD_DIR := build

# SDL2 paths
SDL2_DIR := $(VENDOR)/SDL2-2.32.10
ARCH     := x86_64-w64-mingw32

SDL2_INC := -I$(SDL2_DIR)/$(ARCH)/include/SDL2
SDL2_LIB := -L$(SDL2_DIR)/$(ARCH)/lib

# SDL2_ttf (from system MinGW)
SDL2TTF_INC := -IC:/msys64/mingw64/include/SDL2
SDL2TTF_LIB := -LC:/msys64/mingw64/lib

# ImGui
IMGUI_DIR := $(VENDOR)/imgui

SRC_DIR := src
SRCS    := $(wildcard $(SRC_DIR)/*.cpp)

IMGUI_SRCS := \
    $(IMGUI_DIR)/imgui.cpp \
    $(IMGUI_DIR)/imgui_draw.cpp \
    $(IMGUI_DIR)/imgui_tables.cpp \
    $(IMGUI_DIR)/imgui_widgets.cpp \
    $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp \
    $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp

ALL_SRCS := $(SRCS) $(IMGUI_SRCS)

INCS := $(SDL2_INC) $(SDL2TTF_INC) -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
LIBS := $(SDL2TTF_LIB) $(SDL2_LIB) -Wl,--start-group -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -Wl,--end-group -mwindows

TARGET := $(BUILD_DIR)/moptracker.exe

.PHONY: all clean run

all: $(TARGET)

$(BUILD_DIR)/moptracker.exe: $(ALL_SRCS) Makefile | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCS) $(ALL_SRCS) $(LIBS) -o $(TARGET)
	powershell -NoProfile -Command "Copy-Item -LiteralPath '$(VENDOR)/dll/SDL2.dll' -Destination '$(BUILD_DIR)/'"
	powershell -NoProfile -Command "Copy-Item -LiteralPath '$(VENDOR)/dll/SDL2_ttf.dll' -Destination '$(BUILD_DIR)/'"

$(BUILD_DIR):
	mkdir $(BUILD_DIR) 2>NUL

clean:
	rmdir /S /Q $(BUILD_DIR) 2>NUL

run: all
	$(TARGET)
