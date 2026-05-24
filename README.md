# MopTracker - Time Tracking Desktop Application

A desktop time tracker written in C++ using SDL2, SDL\_ttf, and Dear ImGui.

## Features

- Track time across multiple tasks with a single click
- Visual task buttons with live elapsed time display
- Automatic color assignment via golden-angle HSV distribution
- Dark/Light mode toggle
- IT/EN language toggle
- Monthly statistics view with graphical timeline and hour-range filter
- Manual interval editing (start/end time, task reassignment)
- Drag & drop task reordering
- Task hiding and soft-delete
- Separate OS window for statistics (Windows)
- Auto-save every 10 seconds for crash resilience
- Single JSON file persistence (`moptracker.json`)

## Dependencies

### Required
- **C++17** compatible compiler (g++, clang++, MSVC)
- **SDL2** >= 2.0.10 (vendor/SDL2-2.32.10)
- **SDL2\_ttf** >= 2.0.15 (system MinGW)
- **Dear ImGui** >= 1.90 (vendor/imgui, with SDL2 + SDL\_Renderer backends)

## Building

### Windows (MinGW-w64 with MSYS2)

```bash
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-gcc
make
```

### Linux
```bash
sudo apt install libsdl2-dev libsdl2-ttf-dev g++
make
```

### Manual compilation
```bash
g++ -std=c++17 -O2 -Wall $(sdl2-config --cflags) \
    -Ivendor/imgui -Ivendor/imgui/backends \
    src/*.cpp vendor/imgui/*.cpp vendor/imgui/backends/imgui_impl_sdl2.cpp vendor/imgui/backends/imgui_impl_sdlrenderer2.cpp \
    $(sdl2-config --libs) -lSDL2_ttf -o moptracker
```

## Project Structure

```
moptracker/
├── src/
│   ├── main.cpp          # Entry point, SDL2 + ImGui initialization
│   ├── gui.h/cpp         # Main GUI rendering (task buttons, top bar, dialogs)
│   ├── stats.h/cpp       # Statistics screen (timeline, interval editing)
│   ├── json.h/cpp        # Minimal JSON parser/serializer
│   ├── types.h           # Shared data types (Task, Interval, Settings)
│   ├── utils.h/cpp       # Time utility functions
│   ├── color.h/cpp       # HSV color distribution
│   ├── tasks.h/cpp       # Task management (CRUD, ordering)
│   ├── intervals.h/cpp   # Interval management (tracking, splitting)
│   └── persistence.h/cpp # JSON file save/load
├── vendor/
│   ├── SDL2-2.32.10/     # SDL2 development files
│   └── imgui/            # Dear ImGui library
├── Makefile
└── README.md
```

## Data File

All data is saved to `moptracker.json` in the same directory as the executable.

## Usage

1. Launch the application
2. Click **+** to add tasks
3. Click a task button to start tracking (click again to stop)
4. Use **Dark** / **Light** to toggle theme
5. Use **IT** / **EN** to switch interface language
6. Open **Statistics** for monthly timeline, interval editing, and hour-range filter
7. Drag & drop task buttons to reorder
