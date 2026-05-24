#pragma once
#include "types.h"
#include "tasks.h"
#include "intervals.h"
#include <string>

class Persistence {
public:
    static std::string filepath;
    static void save(const Settings& settings, const TaskManager& tm, IntervalManager& im);
    static bool load(Settings& settings, TaskManager& tm, IntervalManager& im);
private:
    static std::string get_path();
};
