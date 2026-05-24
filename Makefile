# MopTracker Makefile
#
# Set VENDOR_DIR to the vendor directory containing SDL2 and imgui.
#
# Targets:
#   make          - Build
#   make run      - Build and run
#   make clean    - Remove build artifacts

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
LDFLAGS  ?=

CURDIR   := $(CURDIR)
VENDOR   ?= $(CURDIR)/vendor

# SDL2 paths
SDL2_DIR := $(VENDOR)/SDL2-2.32.10
ARCH     := x86_64-w64-mingw32

# SDL2
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

TARGET := moptracker.exe

$(TARGET): $(ALL_SRCS) Makefile
	$(CXX) $(CXXFLAGS) $(INCS) $(ALL_SRCS) $(LIBS) -o $(TARGET)

.PHONY: clean run

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
