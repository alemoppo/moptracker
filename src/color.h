#pragma once
#include <cstdint>

void hsv_to_rgb(float h, float s, float v, float& r, float& g, float& b);
void assign_task_color(float* r, float* g, float* b, int index, int total);
