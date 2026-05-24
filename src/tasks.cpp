#include "tasks.h"
#include "color.h"

Task* TaskManager::find(uint64_t id) {
    for (auto& t : tasks)
        if (t.id == id) return &t;
    return nullptr;
}

const Task* TaskManager::find(uint64_t id) const {
    for (auto& t : tasks)
        if (t.id == id) return &t;
    return nullptr;
}

uint64_t TaskManager::add_task(const std::string& name) {
    uint64_t id = next_id++;
    Task t;
    t.id = id;
    t.name = name;
    t.order = static_cast<int>(tasks.size());
    assign_task_color(&t.color_r, &t.color_g, &t.color_b, visible_count(), visible_count() + 1);
    tasks.push_back(t);
    return id;
}

void TaskManager::rename_task(uint64_t id, const std::string& new_name) {
    Task* t = find(id);
    if (t) t->name = new_name;
}

void TaskManager::set_task_color(uint64_t id, float r, float g, float b) {
    Task* t = find(id);
    if (t) { t->color_r = r; t->color_g = g; t->color_b = b; }
}

void TaskManager::set_task_hidden(uint64_t id, bool hidden) {
    Task* t = find(id);
    if (t) t->hidden = hidden;
}

void TaskManager::delete_task(uint64_t id) {
    Task* t = find(id);
    if (t) t->deleted = true;
}

void TaskManager::reorder_task(uint64_t id, int new_order) {
    Task* t = find(id);
    if (!t) return;
    int old_order = t->order;
    if (new_order == old_order) return;
    if (new_order > old_order) {
        for (auto& task : tasks) {
            if (task.deleted) continue;
            if (task.order > old_order && task.order <= new_order)
                task.order--;
        }
    } else {
        for (auto& task : tasks) {
            if (task.deleted) continue;
            if (task.order >= new_order && task.order < old_order)
                task.order++;
        }
    }
    t->order = new_order;
    reassign_orders();
}

void TaskManager::reassign_orders() {
    std::vector<Task*> sorted;
    for (auto& t : tasks)
        if (!t.deleted) sorted.push_back(&t);
    std::sort(sorted.begin(), sorted.end(), [](const Task* a, const Task* b) {
        return a->order < b->order;
    });
    for (int i = 0; i < (int)sorted.size(); i++)
        sorted[i]->order = i;
}

std::vector<Task*> TaskManager::visible_tasks() {
    std::vector<Task*> result;
    for (auto& t : tasks) {
        if (!t.hidden && !t.deleted) result.push_back(&t);
    }
    std::sort(result.begin(), result.end(), [](const Task* a, const Task* b) {
        return a->order < b->order;
    });
    return result;
}

int TaskManager::visible_count() const {
    int count = 0;
    for (auto& t : tasks)
        if (!t.hidden && !t.deleted) count++;
    return count;
}
