#include "stats.h"
#include "gui.h"
#include "utils.h"
#include "imgui.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

// Filter defaults
static constexpr int FILTER_INIT_END    = 24;
static constexpr int FILTER_HOUR_MIN    = 0;
static constexpr int FILTER_HOUR_MAX_ST = 23;
static constexpr int FILTER_HOUR_MIN_EN = 1;
static constexpr int FILTER_HOUR_MAX_EN = 24;

// Stats frame style
static constexpr float STATS_FRAME_ROUND = 4.0f;
static constexpr float FILTER_SPACING     = 30.0f;

// Day bar
static constexpr float DAY_BAR_H        = 40.0f;
static constexpr float DAY_BAR_ROUND    = 4.0f;
static constexpr int   DAY_BAR_BG[4]    = {60, 60, 60, 100};

// Hour labels / separators
static constexpr int   HOUR_LABEL_STEP  = 3;
static constexpr int   HOUR_LABEL_BUF   = 8;
static constexpr int   HOUR_LINE_COL[4] = {200, 200, 200, 80};
static constexpr int   HOUR_TEXT_COL[4] = {180, 180, 180, 200};

// Day bar clickable area extra
static constexpr float BAR_CLICK_EXTRA  = 20.0f;

// Interval rows
static constexpr float INDENT_W         = 10.0f;
static constexpr float UNINDENT_W       = 10.0f;
static constexpr int   PUSHID_DAY_MULT  = 1000;
static constexpr float COLOR_BTN_SZ     = 12.0f;
static constexpr float MIN_COMBO_W      = 60.0f;
static constexpr float COMBO_EXTRA      = 30.0f;

// Time input fields
static constexpr int   TIME_BUF         = 9;
static constexpr float TIME_INPUT_W     = 80.0f;
static constexpr int   TIME_MIN_MATCH   = 2;
static constexpr int   TIME_MAX_H       = 23;
static constexpr int   TIME_MAX_M       = 59;
static constexpr int   TIME_MAX_S       = 59;
static constexpr int   TIME_MIN_VAL     = 0;

// Interval bar rendering
static constexpr int   COL_MULT         = 255;
static constexpr int   BAR_ALPHA        = 200;
static constexpr float BAR_ROUND        = 3.0f;

// Add interval defaults
static constexpr int   DEFAULT_INTERVAL_SEC = 3600;

// Task management table
static constexpr float DUMMY_SPACE      = 10.0f;
static constexpr int   TABLE_COLS       = 5;

// PushID offsets for task management controls
static constexpr int   VIS_OFFSET       = 10000;
static constexpr int   RENAME_OFFSET    = 20000;
static constexpr float RENAME_INPUT_W   = 150.0f;
static constexpr int   RENAME_BUF       = 256;
static constexpr int   DELETE_OFFSET    = 30000;

// Stats window default size
static constexpr float STATS_DEF_W      = 950.0f;
static constexpr float STATS_DEF_H      = 700.0f;

static int stats_year = 0;
static int stats_month = 0;
static int filter_hour_start = 0;
static int filter_hour_end   = FILTER_INIT_END;

static int find_orig_interval(const std::vector<Interval>& all, const Interval& displayed) {
    for (size_t i = 0; i < all.size(); i++) {
        const auto& o = all[i];
        int64_t o_end = o.end_ts == 0 ? unix_now() : o.end_ts;
        if (o.task_id == displayed.task_id && o.start_ts <= displayed.start_ts && o_end >= displayed.end_ts)
            return (int)i;
    }
    return -1;
}

void render_stats_content(AppGUI* app) {
    if (!app->show_stats) return;

    filter_hour_start = app->settings.filter_hour_start;
    filter_hour_end   = app->settings.filter_hour_end;

    if (stats_year == 0 || stats_month == 0) {
        int64_t now = unix_now();
        struct tm lt = local_time(now);
        stats_year = lt.tm_year + EPOCH_YEAR_OFFSET;
        stats_month = lt.tm_mon + TM_MON_ADJUST;
    }

    int64_t now_ts = unix_now();
    struct tm now_tm = local_time(now_ts);

    // Top bar: nav + filter
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, STATS_FRAME_ROUND);

        if (ImGui::ArrowButton("##p", ImGuiDir_Left)) {
            stats_month--;
            if (stats_month < 1) { stats_month = 12; stats_year--; }
        }
        ImGui::SameLine();
        ImGui::Text("  %s  ", format_month(stats_year, stats_month, app->settings.language).c_str());
        ImGui::SameLine();
        if (ImGui::ArrowButton("##n", ImGuiDir_Right)) {
            stats_month++;
            if (stats_month > 12) { stats_month = 1; stats_year++; }
        }

        ImGui::SameLine(0, FILTER_SPACING);

        ImGui::Text("%s", app->tr("Show:", "Mostra:"));
        ImGui::SameLine();
        if (ImGui::ArrowButton("##fld", ImGuiDir_Left)) {
            filter_hour_start = (std::max)(FILTER_HOUR_MIN, filter_hour_start - 1);
            if (filter_hour_start >= filter_hour_end) filter_hour_end = filter_hour_start + 1;
        }
        ImGui::SameLine();
        ImGui::Text("%02d", filter_hour_start);
        ImGui::SameLine();
        if (ImGui::ArrowButton("##fru", ImGuiDir_Right)) {
            filter_hour_start = (std::min)(FILTER_HOUR_MAX_ST, filter_hour_start + 1);
            if (filter_hour_start >= filter_hour_end) filter_hour_end = filter_hour_start + 1;
        }
        ImGui::SameLine();
        ImGui::Text("%s", app->tr("to", "a"));
        ImGui::SameLine();
        if (ImGui::ArrowButton("##fld2", ImGuiDir_Left)) {
            filter_hour_end = (std::max)(FILTER_HOUR_MIN_EN, filter_hour_end - 1);
            if (filter_hour_end <= filter_hour_start) filter_hour_start = filter_hour_end - 1;
        }
        ImGui::SameLine();
        ImGui::Text("%02d", filter_hour_end);
        ImGui::SameLine();
        if (ImGui::ArrowButton("##fru2", ImGuiDir_Right)) {
            filter_hour_end = (std::min)(FILTER_HOUR_MAX_EN, filter_hour_end + 1);
            if (filter_hour_end <= filter_hour_start) filter_hour_start = filter_hour_end - 1;
        }

        if (filter_hour_start != app->settings.filter_hour_start ||
            filter_hour_end != app->settings.filter_hour_end) {
            app->settings.filter_hour_start = filter_hour_start;
            app->settings.filter_hour_end   = filter_hour_end;
            app->save_state();
        }

        ImGui::PopStyleVar();
    }

    ImGui::Separator();
    ImGui::BeginChild("sc", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    int days = days_in_month(stats_year, stats_month);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    int filter_span = filter_hour_end - filter_hour_start;

    for (int day = 1; day <= days; day++) {
        if (stats_year > now_tm.tm_year + EPOCH_YEAR_OFFSET ||
            (stats_year == now_tm.tm_year + EPOCH_YEAR_OFFSET && stats_month > now_tm.tm_mon + TM_MON_ADJUST) ||
            (stats_year == now_tm.tm_year + EPOCH_YEAR_OFFSET && stats_month == now_tm.tm_mon + TM_MON_ADJUST && day > now_tm.tm_mday)) {
            continue;
        }

        int64_t ds = make_timestamp(stats_year, stats_month, day, 0, 0, 0);
        int64_t de = make_timestamp(stats_year, stats_month, day, END_OF_DAY_HOUR, END_OF_DAY_MINUTE, END_OF_DAY_SECOND);

        auto raw = app->interval_mgr.intervals_for_day(ds, de);
        std::vector<Interval> filtered;
        for (auto& inv : raw) {
            const Task* t = app->task_mgr.find(inv.task_id);
            if (t && !t->deleted) filtered.push_back(inv);
        }

        if (filtered.empty()) continue;

        ImGui::PushID(day);

        char hdr[32];
        snprintf(hdr, sizeof(hdr), "%s %d - %s", app->tr("Day", "Giorno"), day, format_date(ds).c_str());
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", hdr);

        ImVec2 tl = ImGui::GetCursorScreenPos();
        float tw = ImGui::GetContentRegionAvail().x;
        dl->AddRectFilled(ImVec2(tl.x, tl.y), ImVec2(tl.x + tw, tl.y + DAY_BAR_H), IM_COL32(DAY_BAR_BG[0], DAY_BAR_BG[1], DAY_BAR_BG[2], DAY_BAR_BG[3]), DAY_BAR_ROUND);

        int64_t filter_start_sec = filter_hour_start * SECONDS_PER_HOUR;
        int64_t filter_end_sec   = filter_hour_end * SECONDS_PER_HOUR;
        int64_t span_sec = filter_end_sec - filter_start_sec;
        if (span_sec <= 0) span_sec = SECONDS_PER_DAY;

        for (auto& inv : filtered) {
            int64_t s = std::max(inv.start_ts, ds + filter_start_sec);
            int64_t e = std::min(inv.end_ts == 0 ? unix_now() : inv.end_ts, ds + filter_end_sec);
            if (e <= s) continue;
            float x1 = tl.x + (float)(s - ds - filter_start_sec) / (float)span_sec * tw;
            float x2 = tl.x + (float)(e - ds - filter_start_sec) / (float)span_sec * tw;
            const Task* t = app->task_mgr.find(inv.task_id);
            if (t) {
                ImU32 col = IM_COL32((int)(t->color_r*COL_MULT), (int)(t->color_g*COL_MULT), (int)(t->color_b*COL_MULT), BAR_ALPHA);
                dl->AddRectFilled(ImVec2(x1, tl.y+2), ImVec2(x2, tl.y+DAY_BAR_H-2), col, BAR_ROUND);
            }
        }
        for (int h = filter_hour_start; h <= filter_hour_end; h += HOUR_LABEL_STEP) {
            float x = tl.x + (float)(h - filter_hour_start) / (float)filter_span * tw;
            char lb[HOUR_LABEL_BUF]; snprintf(lb, sizeof(lb), "%02d:00", h);
            dl->AddLine(ImVec2(x, tl.y), ImVec2(x, tl.y + DAY_BAR_H), IM_COL32(HOUR_LINE_COL[0], HOUR_LINE_COL[1], HOUR_LINE_COL[2], HOUR_LINE_COL[3]));
            dl->AddText(ImVec2(x+2, tl.y+DAY_BAR_H+2), IM_COL32(HOUR_TEXT_COL[0], HOUR_TEXT_COL[1], HOUR_TEXT_COL[2], HOUR_TEXT_COL[3]), lb);
        }
        ImGui::SetCursorScreenPos(ImVec2(tl.x, tl.y));
        ImGui::InvisibleButton("tl", ImVec2(tw, DAY_BAR_H + BAR_CLICK_EXTRA));

        ImGui::Indent(INDENT_W);

        for (size_t i = 0; i < filtered.size(); i++) {
            auto& inv = filtered[i];
            const Task* task = app->task_mgr.find(inv.task_id);
            int ri = find_orig_interval(app->interval_mgr.intervals, inv);

            ImGui::PushID((int)(i + day * PUSHID_DAY_MULT));

            if (task) {
                ImGui::ColorButton("##c", ImVec4(task->color_r, task->color_g, task->color_b, 1.0f),
                    ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker, ImVec2(COLOR_BTN_SZ, COLOR_BTN_SZ));
                ImGui::SameLine();
            }

            // Calculate max task name width for combo sizing
            float max_task_w = MIN_COMBO_W;
            for (auto& t : app->task_mgr.tasks) {
                if (t.deleted) continue;
                float tw2 = ImGui::CalcTextSize(t.name.c_str()).x;
                if (tw2 > max_task_w) max_task_w = tw2;
            }
            std::string cname = task ? task->name : app->tr("Unknown", "Sconosciuto");
            ImGui::SetNextItemWidth(max_task_w + COMBO_EXTRA);
            if (ImGui::BeginCombo("##t", cname.c_str())) {
                for (auto& t : app->task_mgr.tasks) {
                    if (t.deleted) continue;
                    bool sel = (t.id == inv.task_id);
                    if (ImGui::Selectable(t.name.c_str(), sel) && ri >= 0) {
                        Interval m = app->interval_mgr.intervals[ri];
                        m.task_id = t.id;
                        app->interval_mgr.update_interval(ri, m);
                        app->save_state();
                    }
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();

            // Start time
            struct tm stm = local_time(inv.start_ts);
            char st_buf[TIME_BUF];
            snprintf(st_buf, sizeof(st_buf), "%02d:%02d:%02d", stm.tm_hour, stm.tm_min, stm.tm_sec);
            ImGui::SetNextItemWidth(TIME_INPUT_W);
            if (ImGui::InputText("##st", st_buf, sizeof(st_buf), ImGuiInputTextFlags_EnterReturnsTrue) && ri >= 0) {
                int h, m, s = 0;
                if (sscanf(st_buf, "%d:%d:%d", &h, &m, &s) >= TIME_MIN_MATCH && h >= TIME_MIN_VAL && h <= TIME_MAX_H && m >= TIME_MIN_VAL && m <= TIME_MAX_M && s >= TIME_MIN_VAL && s <= TIME_MAX_S) {
                    int64_t ns = make_timestamp(stm.tm_year+EPOCH_YEAR_OFFSET, stm.tm_mon+TM_MON_ADJUST, stm.tm_mday, h, m, s);
                    Interval mi = app->interval_mgr.intervals[ri];
                    mi.start_ts = ns;
                    if (mi.end_ts == 0 || mi.end_ts >= mi.start_ts) {
                        app->interval_mgr.update_interval(ri, mi);
                        app->save_state();
                    }
                }
            }

            ImGui::SameLine(); ImGui::Text("-"); ImGui::SameLine();

            // End time
            int64_t ev = inv.end_ts == 0 ? unix_now() : inv.end_ts;
            struct tm etm = local_time(ev);
            char et_buf[TIME_BUF];
            snprintf(et_buf, sizeof(et_buf), "%02d:%02d:%02d", etm.tm_hour, etm.tm_min, etm.tm_sec);
            ImGui::SetNextItemWidth(TIME_INPUT_W);
            if (ImGui::InputText("##et", et_buf, sizeof(et_buf), ImGuiInputTextFlags_EnterReturnsTrue) && ri >= 0) {
                int h, m, s = 0;
                if (sscanf(et_buf, "%d:%d:%d", &h, &m, &s) >= TIME_MIN_MATCH && h >= TIME_MIN_VAL && h <= TIME_MAX_H && m >= TIME_MIN_VAL && m <= TIME_MAX_M && s >= TIME_MIN_VAL && s <= TIME_MAX_S) {
                    int64_t ne = make_timestamp(etm.tm_year+EPOCH_YEAR_OFFSET, etm.tm_mon+TM_MON_ADJUST, etm.tm_mday, h, m, s);
                    if (ne >= app->interval_mgr.intervals[ri].start_ts) {
                        Interval mi = app->interval_mgr.intervals[ri];
                        mi.end_ts = ne;
                        app->interval_mgr.update_interval(ri, mi);
                        app->save_state();
                    }
                }
            }

            ImGui::SameLine();

            int64_t dur = (inv.end_ts == 0 ? unix_now() : inv.end_ts) - inv.start_ts;
            if (dur > 0) ImGui::Text("(%s)", format_duration(dur).c_str());
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f,0.1f,0.1f,0.3f));
            if (ImGui::Button("X") && ri >= 0) {
                app->interval_mgr.remove_interval(ri);
                app->save_state();
            }
            ImGui::PopStyleColor();
            ImGui::PopID();
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f,0.6f,0.2f,0.3f));
        if (ImGui::SmallButton(app->tr("+ Add Interval", "+ Aggiungi Intervallo"))) {
            auto vis = app->task_mgr.visible_tasks();
            if (!vis.empty()) {
                Interval ni;
                ni.task_id = vis[0]->id;
                ni.start_ts = ds + filter_hour_start * SECONDS_PER_HOUR;
                ni.end_ts = ni.start_ts + DEFAULT_INTERVAL_SEC;
                app->interval_mgr.add_interval(ni);
                app->save_state();
            }
        }
        ImGui::PopStyleColor();

        ImGui::Unindent(UNINDENT_W);
        ImGui::Separator();
        ImGui::PopID();
    }

    // Task management table
    ImGui::Dummy(ImVec2(0, DUMMY_SPACE));
    ImGui::Separator();
    ImGui::Text("%s", app->tr("Task Management", "Gestione Task"));

    if (ImGui::BeginTable("tbl", TABLE_COLS, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn(app->tr("Name", "Nome"));
        ImGui::TableSetupColumn(app->tr("Color", "Colore"));
        ImGui::TableSetupColumn(app->tr("Visible", "Visibile"));
        ImGui::TableSetupColumn(app->tr("Rename", "Rinomina"));
        ImGui::TableSetupColumn(app->tr("Delete", "Elimina"));
        ImGui::TableHeadersRow();

        for (auto& task : app->task_mgr.tasks) {
            if (task.deleted) continue;
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", task.name.c_str());

            ImGui::TableNextColumn();
            float col[3] = { task.color_r, task.color_g, task.color_b };
            ImGui::PushID((int)task.id);
            if (ImGui::ColorEdit3("##c", col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
                task.color_r = col[0]; task.color_g = col[1]; task.color_b = col[2];
                app->save_state();
            }
            ImGui::PopID();

            ImGui::TableNextColumn();
            bool vis = !task.hidden;
            ImGui::PushID((int)(task.id + VIS_OFFSET));
            if (ImGui::Checkbox("##v", &vis)) { task.hidden = !vis; app->save_state(); }
            ImGui::PopID();

            ImGui::TableNextColumn();
            ImGui::PushID((int)(task.id + RENAME_OFFSET));
            ImGui::SetNextItemWidth(RENAME_INPUT_W);
            char rnbuf[RENAME_BUF];
            strncpy(rnbuf, task.name.c_str(), sizeof(rnbuf) - 1);
            rnbuf[sizeof(rnbuf) - 1] = '\0';
            if (ImGui::InputText("##rn", rnbuf, sizeof(rnbuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (rnbuf[0]) { task.name = rnbuf; app->save_state(); }
            }
            ImGui::PopID();

            ImGui::TableNextColumn();
            ImGui::PushID((int)(task.id + DELETE_OFFSET));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f,0.1f,0.1f,0.3f));
            if (ImGui::SmallButton("X")) { app->task_mgr.delete_task(task.id); app->save_state(); }
            ImGui::PopStyleColor();
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    ImGui::EndChild();
}

void render_stats_window(AppGUI* app) {
    if (!app->show_stats) return;

    ImGui::SetNextWindowSize(ImVec2(STATS_DEF_W, STATS_DEF_H), ImGuiCond_FirstUseEver);
    ImGui::Begin(app->tr("Statistics", "Statistiche"), &app->show_stats, ImGuiWindowFlags_NoCollapse);
    render_stats_content(app);
    ImGui::End();
}
