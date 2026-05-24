#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <ctime>

static constexpr float  DEF_COLOR        = 0.5f;
static constexpr int    DEF_WIN_X        = 100;
static constexpr int    DEF_WIN_Y        = 100;
static constexpr int    DEF_WIN_W        = 800;
static constexpr int    DEF_WIN_H        = 600;
static constexpr int    DEF_FILTER_START = 0;
static constexpr int    DEF_FILTER_END   = 24;

struct Task {
    uint64_t    id = 0;
    std::string name;
    float       color_r = DEF_COLOR;
    float       color_g = DEF_COLOR;
    float       color_b = DEF_COLOR;
    bool        hidden = false;
    int         order = 0;
    bool        deleted = false;
};

struct Interval {
    uint64_t task_id   = 0;
    int64_t  start_ts  = 0;
    int64_t  end_ts    = 0;
};

struct Settings {
    bool dark_mode     = true;
    int  language      = 0; // 0=EN, 1=IT
    int  window_x      = DEF_WIN_X;
    int  window_y      = DEF_WIN_Y;
    int  window_w      = DEF_WIN_W;
    int  window_h      = DEF_WIN_H;
    int  filter_hour_start = DEF_FILTER_START;
    int  filter_hour_end   = DEF_FILTER_END;
};
