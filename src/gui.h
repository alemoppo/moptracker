#pragma once
#include "tasks.h"
#include "intervals.h"
#include "types.h"
#include <string>

class AppGUI;

void render_stats_window(AppGUI* app);

class AppGUI {
public:
    Settings settings;
    TaskManager task_mgr;
    IntervalManager interval_mgr;

    const char* tr(const char* en, const char* it) const {
        return settings.language == 1 ? it : en;
    }

    bool show_stats = false;
    int64_t last_save = 0;
    int64_t last_tick_save = 0;

    AppGUI();
    void init();
    void shutdown();
    void render();
    void periodic_save(int64_t now);
    void save_state();
    void load_state();

private:
    void render_main_window();
    void render_add_task_dialog();
    void render_task_button(Task& task, float width);

    bool show_add_dialog = false;
    static constexpr int NEW_TASK_BUF = 256;
    char new_task_name[NEW_TASK_BUF] = "";
};
