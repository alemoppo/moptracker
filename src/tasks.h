#pragma once
#include "types.h"
#include <vector>
#include <algorithm>

class TaskManager {
public:
    std::vector<Task> tasks;
    uint64_t next_id = 1;

    Task* find(uint64_t id);
    const Task* find(uint64_t id) const;

    uint64_t add_task(const std::string& name);
    void rename_task(uint64_t id, const std::string& new_name);
    void set_task_color(uint64_t id, float r, float g, float b);
    void set_task_hidden(uint64_t id, bool hidden);
    void delete_task(uint64_t id);
    void reorder_task(uint64_t id, int new_order);
    void reassign_orders();

    std::vector<Task*> visible_tasks();
    int visible_count() const;
};
