#include "color.h"
#include <cmath>

static constexpr float GOLDEN_ANGLE   = 137.508f;
static constexpr float HUE_FULL_CIRCLE = 360.0f;
static constexpr float HUE_SEGMENT     = 60.0f;
static constexpr float TASK_SATURATION = 0.65f;
static constexpr float TASK_VALUE      = 0.85f;

void hsv_to_rgb(float h, float s, float v, float& r, float& g, float& b) {
    h = std::fmod(h, HUE_FULL_CIRCLE);
    if (h < 0) h += HUE_FULL_CIRCLE;
    float c = v * s;
    float hp = h / HUE_SEGMENT;
    float x = c * (1.0f - std::fabs(std::fmod(hp, 2.0f) - 1.0f));
    float m = v - c;

    if (hp < 1.0f)      { r = c; g = x; b = 0; }
    else if (hp < 2.0f) { r = x; g = c; b = 0; }
    else if (hp < 3.0f) { r = 0; g = c; b = x; }
    else if (hp < 4.0f) { r = 0; g = x; b = c; }
    else if (hp < 5.0f) { r = x; g = 0; b = c; }
    else                { r = c; g = 0; b = x; }

    r += m; g += m; b += m;
}

void assign_task_color(float* r, float* g, float* b, int index, int total) {
    (void)total;
    float hue = index * GOLDEN_ANGLE;
    hsv_to_rgb(hue, TASK_SATURATION, TASK_VALUE, *r, *g, *b);
}
