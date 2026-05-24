#pragma once
#include "types.h"
#include <vector>
#include <cstdint>

class IntervalManager {
public:
    std::vector<Interval> intervals;
    uint64_t active_task_id = 0;
    int64_t  active_start_ts = 0;

    void start_task(uint64_t task_id, int64_t now);
    void stop_task(int64_t now);
    void toggle_task(uint64_t task_id, int64_t now);

    int64_t elapsed_today(uint64_t task_id) const;
    int64_t elapsed_on_day(uint64_t task_id, int64_t day_start, int64_t day_end) const;
    std::vector<Interval> intervals_for_day(int64_t day_start, int64_t day_end) const;

    static void split_midnight(std::vector<Interval>& intervals);

    void add_interval(const Interval& inv);
    void remove_interval(size_t index);
    void update_interval(size_t index, const Interval& inv);
    void cleanup_old(int64_t now);

    Interval* active_interval();
    const Interval* active_interval() const;
    void close_active_interval(int64_t now);
    int64_t current_active_elapsed(int64_t now) const;
    void merge_contiguous();
};
