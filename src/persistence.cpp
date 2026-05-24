#include "persistence.h"
#include "json.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <climits>
#endif

static constexpr int PATH_BUF_SZ       = 1024;
static constexpr int JSON_INDENT       = 2;

std::string Persistence::filepath = "";

std::string Persistence::get_path() {
    if (!filepath.empty()) return filepath;
    char buf[PATH_BUF_SZ];
    buf[0] = '\0';
#ifdef _WIN32
    GetModuleFileNameA(nullptr, buf, sizeof(buf));
#else
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) buf[len] = '\0';
#endif
    std::string exe_path(buf);
    size_t sep = exe_path.find_last_of("/\\");
    std::string dir = (sep != std::string::npos) ? exe_path.substr(0, sep + 1) : "./";
    filepath = dir + "moptracker.json";
    return filepath;
}

void Persistence::save(const Settings& settings, const TaskManager& tm, IntervalManager& im) {
    Json root;
    Json s;
    s["dark_mode"] = settings.dark_mode;
    s["language"]  = settings.language;
    s["window_x"]  = settings.window_x;
    s["window_y"]  = settings.window_y;
    s["window_w"]  = settings.window_w;
    s["window_h"]  = settings.window_h;
    s["filter_hour_start"] = settings.filter_hour_start;
    s["filter_hour_end"]   = settings.filter_hour_end;
    root["settings"] = s;

    Json tasks_arr;
    for (auto& t : tm.tasks) {
        Json tj;
        tj["id"]      = static_cast<int64_t>(t.id);
        tj["name"]    = t.name;
        tj["color_r"] = t.color_r;
        tj["color_g"] = t.color_g;
        tj["color_b"] = t.color_b;
        tj["hidden"]  = t.hidden;
        tj["order"]   = t.order;
        tj["deleted"] = t.deleted;
        tasks_arr.push_back(tj);
    }
    root["tasks"] = tasks_arr;

    Json intervals_arr;
    for (auto& inv : im.intervals) {
        Json ij;
        ij["task_id"]  = static_cast<int64_t>(inv.task_id);
        ij["start_ts"] = inv.start_ts;
        ij["end_ts"]   = inv.end_ts;
        intervals_arr.push_back(ij);
    }
    root["intervals"] = intervals_arr;

    std::ofstream file(get_path());
    if (file.is_open()) {
        file << root.dump(JSON_INDENT);
        file.close();
    } else {
        std::cerr << "Error: Could not write to " << get_path() << std::endl;
    }
}

bool Persistence::load(Settings& settings, TaskManager& tm, IntervalManager& im) {
    std::ifstream file(get_path());
    if (!file.is_open()) return false;

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    if (content.empty()) return false;

    try {
        Json root = Json::parse(content);

        if (root.has("settings")) {
            auto& s = root["settings"];
            if (s.has("dark_mode"))  settings.dark_mode = s["dark_mode"].as_bool();
            if (s.has("language"))   settings.language  = (int)s["language"].as_int();
            if (s.has("window_x"))   settings.window_x  = (int)s["window_x"].as_int();
            if (s.has("window_y"))   settings.window_y  = (int)s["window_y"].as_int();
            if (s.has("window_w"))   settings.window_w  = (int)s["window_w"].as_int();
            if (s.has("window_h"))   settings.window_h  = (int)s["window_h"].as_int();
            if (s.has("filter_hour_start")) settings.filter_hour_start = (int)s["filter_hour_start"].as_int();
            if (s.has("filter_hour_end"))   settings.filter_hour_end   = (int)s["filter_hour_end"].as_int();
        }

        if (root.has("tasks")) {
            auto& arr = root["tasks"];
            tm.tasks.clear();
            uint64_t max_id = 0;
            for (size_t i = 0; i < arr.size(); i++) {
                auto& tj = arr[i];
                Task t;
                t.id      = (uint64_t)tj["id"].as_int();
                t.name    = tj["name"].as_string();
                t.color_r = (float)tj["color_r"].as_number();
                t.color_g = (float)tj["color_g"].as_number();
                t.color_b = (float)tj["color_b"].as_number();
                t.hidden  = tj["hidden"].as_bool();
                t.order   = (int)tj["order"].as_int();
                t.deleted = tj.has("deleted") ? tj["deleted"].as_bool() : false;
                tm.tasks.push_back(t);
                if (t.id > max_id) max_id = t.id;
            }
            tm.next_id = max_id + 1;
        }

        if (root.has("intervals")) {
            auto& arr = root["intervals"];
            im.intervals.clear();
            for (size_t i = 0; i < arr.size(); i++) {
                auto& ij = arr[i];
                Interval inv;
                inv.task_id  = (uint64_t)ij["task_id"].as_int();
                inv.start_ts = ij["start_ts"].as_int();
                inv.end_ts   = ij["end_ts"].as_int();
                im.intervals.push_back(inv);
            }
            im.merge_contiguous();
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return false;
    }
}
