#include "intervals.h"
#include "utils.h"
#include <algorithm>
#include <cmath>

void IntervalManager::start_task(uint64_t task_id, int64_t now) {
    if (active_task_id != 0) stop_task(now);
    active_task_id = task_id;
    active_start_ts = now;
    Interval inv;
    inv.task_id = task_id;
    inv.start_ts = now;
    inv.end_ts = 0;
    intervals.push_back(inv);
}

void IntervalManager::stop_task(int64_t now) {
    if (active_task_id == 0) return;
    for (auto& inv : intervals) {
        if (inv.task_id == active_task_id && inv.end_ts == 0) {
            inv.end_ts = now;
            break;
        }
    }
    active_task_id = 0;
    active_start_ts = 0;
}

void IntervalManager::toggle_task(uint64_t task_id, int64_t now) {
    if (active_task_id == task_id) {
        stop_task(now);
    } else {
        if (active_task_id != 0) stop_task(now);
        start_task(task_id, now);
    }
}

int64_t IntervalManager::elapsed_today(uint64_t task_id) const {
    int64_t now = unix_now();
    return elapsed_on_day(task_id, day_start_ts(now), day_end_ts(now));
}

int64_t IntervalManager::elapsed_on_day(uint64_t task_id, int64_t day_start, int64_t day_end) const {
    int64_t total = 0;
    for (auto& inv : intervals) {
        if (inv.task_id != task_id) continue;
        int64_t s = std::max(inv.start_ts, day_start);
        int64_t e = inv.end_ts == 0 ? unix_now() : inv.end_ts;
        e = std::min(e, day_end);
        if (e > s) total += (e - s);
    }
    return total;
}

std::vector<Interval> IntervalManager::intervals_for_day(int64_t day_start, int64_t day_end) const {
    std::vector<Interval> result;
    for (auto& inv : intervals) {
        int64_t s = inv.start_ts;
        int64_t e = inv.end_ts == 0 ? unix_now() : inv.end_ts;
        if (s >= day_end || e <= day_start) continue;
        Interval clamped;
        clamped.task_id = inv.task_id;
        clamped.start_ts = std::max(s, day_start);
        clamped.end_ts = std::min(e, day_end);
        result.push_back(clamped);
    }
    split_midnight(result);
    return result;
}

void IntervalManager::split_midnight(std::vector<Interval>& intervals) {
    std::vector<Interval> result;
    for (auto& inv : intervals) {
        int64_t s = inv.start_ts;
        int64_t e = inv.end_ts == 0 ? unix_now() : inv.end_ts;
        int64_t s_ds = day_start_ts(s);
        int64_t e_ds = day_start_ts(e);
        if (s_ds == e_ds) {
            result.push_back(inv);
        } else {
            int64_t current = s;
            while (current < e) {
                int64_t next_midnight = day_end_ts(current) + 1;
                Interval part;
                part.task_id = inv.task_id;
                part.start_ts = current;
                part.end_ts = std::min(next_midnight, e);
                if (part.end_ts > part.start_ts)
                    result.push_back(part);
                current = part.end_ts;
            }
        }
    }
    intervals = result;
}

void IntervalManager::add_interval(const Interval& inv) {
    intervals.push_back(inv);
}

void IntervalManager::remove_interval(size_t index) {
    if (index < intervals.size())
        intervals.erase(intervals.begin() + index);
}

void IntervalManager::update_interval(size_t index, const Interval& inv) {
    if (index < intervals.size())
        intervals[index] = inv;
}

static constexpr int64_t CLEANUP_CUTOFF_SEC = 365LL * 24 * 3600;
static constexpr int64_t MERGE_GAP_THRESHOLD = 10;

void IntervalManager::cleanup_old(int64_t now) {
    int64_t cutoff = now - CLEANUP_CUTOFF_SEC;
    std::vector<Interval> kept;
    for (auto& inv : intervals) {
        if (inv.end_ts == 0 || inv.end_ts > cutoff)
            kept.push_back(inv);
    }
    intervals = kept;
}

Interval* IntervalManager::active_interval() {
    for (auto& inv : intervals)
        if (inv.end_ts == 0) return &inv;
    return nullptr;
}

const Interval* IntervalManager::active_interval() const {
    for (auto& inv : intervals)
        if (inv.end_ts == 0) return &inv;
    return nullptr;
}

void IntervalManager::close_active_interval(int64_t now) {
    Interval* act = active_interval();
    if (act) act->end_ts = now;
    active_task_id = 0;
    active_start_ts = 0;
}

int64_t IntervalManager::current_active_elapsed(int64_t now) const {
    if (active_task_id == 0) return 0;
    return now - active_start_ts;
}

void IntervalManager::merge_contiguous() {
    if (intervals.size() < 2) return;
    std::sort(intervals.begin(), intervals.end(),
        [](const Interval& a, const Interval& b) { return a.start_ts < b.start_ts; });
    std::vector<Interval> merged;
    merged.push_back(intervals[0]);
    for (size_t i = 1; i < intervals.size(); i++) {
        Interval& last = merged.back();
        const Interval& cur = intervals[i];
        if (cur.task_id == last.task_id) {
            int64_t last_end = last.end_ts == 0 ? unix_now() : last.end_ts;
            int64_t cur_start = cur.start_ts;
            int64_t cur_end = cur.end_ts == 0 ? unix_now() : cur.end_ts;
            if (cur_start <= last_end + MERGE_GAP_THRESHOLD) {
                if (cur_end > last_end) {
                    if (last.end_ts == 0) {
                        // Keep active interval active but extend its tracked time
                        // (active interval's end_ts stays 0; start_ts unchanged)
                    } else {
                        last.end_ts = cur_end;
                    }
                }
                continue;
            }
        }
        merged.push_back(cur);
    }
    intervals = merged;
}
