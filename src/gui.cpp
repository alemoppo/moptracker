#include "gui.h"
#include "stats.h"
#include "persistence.h"
#include "utils.h"
#include "imgui.h"
#include <algorithm>

// Style constants
static constexpr float FRAME_ROUNDING     = 6.0f;
static constexpr float GRAB_ROUNDING      = 4.0f;
static constexpr float WINDOW_ROUNDING    = 8.0f;
static constexpr float CHILD_ROUNDING     = 6.0f;
static constexpr float SCROLLBAR_ROUNDING = 4.0f;
static constexpr float FRAME_PAD_X        = 10.0f;
static constexpr float FRAME_PAD_Y        = 6.0f;
static constexpr float ITEM_SPACE_X       = 8.0f;
static constexpr float ITEM_SPACE_Y       = 6.0f;

// Layout constants
static constexpr float TOP_BAR_HEIGHT     = 40.0f;
static constexpr float CORNER_ROUNDING    = 6.0f;
static constexpr float TOP_RIGHT_BTN_W    = 100.0f;
static constexpr float TOP_RIGHT_SPACING  = 6.0f;
static constexpr int   TOP_RIGHT_BTN_N    = 3;

static constexpr float MIN_TASK_BTN_W     = 180.0f;
static constexpr float GRID_PADDING       = 30.0f;
static constexpr float GRID_DIV           = 3.0f;
static constexpr float GRID_GAP           = 10.0f;
static constexpr float TASK_BTN_H         = 80.0f;
static constexpr float ADD_BTN_H          = 80.0f;
static constexpr float ACTIVE_BORDER      = 3.0f;
static constexpr float DIALOG_BTN_W       = 120.0f;

static constexpr float INACTIVE_ALPHA     = 0.5f;
static constexpr int   SAVE_INTERVAL_SEC  = 10;

AppGUI::AppGUI() { init(); }

void AppGUI::init() {
    if (settings.dark_mode)
        ImGui::StyleColorsDark();
    else
        ImGui::StyleColorsLight();

    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = FRAME_ROUNDING;
    style.GrabRounding = GRAB_ROUNDING;
    style.WindowRounding = WINDOW_ROUNDING;
    style.ChildRounding = CHILD_ROUNDING;
    style.ScrollbarRounding = SCROLLBAR_ROUNDING;
    style.FramePadding = ImVec2(FRAME_PAD_X, FRAME_PAD_Y);
    style.ItemSpacing = ImVec2(ITEM_SPACE_X, ITEM_SPACE_Y);
}

void AppGUI::shutdown() {
    int64_t now = unix_now();
    interval_mgr.stop_task(now);
    save_state();
}

void AppGUI::save_state() {
    int64_t now = unix_now();
    interval_mgr.merge_contiguous();
    Interval* act = interval_mgr.active_interval();
    if (act) act->end_ts = now;
    Persistence::save(settings, task_mgr, interval_mgr);
    if (act) act->end_ts = 0;
}

void AppGUI::load_state() {
    Persistence::load(settings, task_mgr, interval_mgr);

    interval_mgr.active_task_id = 0;
    interval_mgr.active_start_ts = 0;

    // Close any orphaned active interval from crash
    const Interval* act = interval_mgr.active_interval();
    if (act) {
        for (size_t i = interval_mgr.intervals.size(); i > 0; i--) {
            auto& inv = interval_mgr.intervals[i - 1];
            if (inv.end_ts == 0) {
                int64_t close_ts = inv.start_ts;
                for (int j = (int)i - 2; j >= 0; j--) {
                    if (interval_mgr.intervals[j].end_ts > close_ts) {
                        close_ts = interval_mgr.intervals[j].end_ts;
                        break;
                    }
                }
                inv.end_ts = close_ts;
                break;
            }
        }
    }

    int64_t now = unix_now();
    interval_mgr.cleanup_old(now);

    if (settings.dark_mode)
        ImGui::StyleColorsDark();
    else
        ImGui::StyleColorsLight();
    init();
}

void AppGUI::periodic_save(int64_t now) {
    if (now - last_tick_save >= SAVE_INTERVAL_SEC) {
        Interval* act = interval_mgr.active_interval();
        if (act) {
            act->end_ts = now;
            Persistence::save(settings, task_mgr, interval_mgr);
            act->end_ts = 0;
        } else {
            Persistence::save(settings, task_mgr, interval_mgr);
        }
        last_tick_save = now;
    }
}

void AppGUI::render() {
    int64_t now = unix_now();
    periodic_save(now);
    render_main_window();
    render_add_task_dialog();
#ifndef _WIN32
    if (show_stats)
        render_stats_window(this);
#endif
}

void AppGUI::render_main_window() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("MopTracker", nullptr, flags);

    // Top bar: draw background rect manually, then lay out items with SameLine
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float win_w = ImGui::GetContentRegionAvail().x;
    ImVec2 p0 = ImGui::GetCursorScreenPos();

    if (interval_mgr.active_task_id != 0) {
        Task* active = task_mgr.find(interval_mgr.active_task_id);
        if (active) {
            ImU32 bg = IM_COL32((int)(active->color_r * 255), (int)(active->color_g * 255),
                                (int)(active->color_b * 255), 40);
            dl->AddRectFilled(p0, ImVec2(p0.x + win_w, p0.y + TOP_BAR_HEIGHT), bg, CORNER_ROUNDING);
        }
    }

    // Active task label on the left
    if (interval_mgr.active_task_id != 0) {
        Task* active = task_mgr.find(interval_mgr.active_task_id);
        if (active) {
            int64_t elapsed = interval_mgr.current_active_elapsed(unix_now());
            std::string es = format_duration(elapsed);
            ImGui::TextColored(ImVec4(active->color_r, active->color_g, active->color_b, 1.0f),
                               "%s %s  [%s]", tr("Active:", "Attivo:"), active->name.c_str(), es.c_str());
        }
    } else {
        ImGui::TextColored(ImVec4(INACTIVE_ALPHA, INACTIVE_ALPHA, INACTIVE_ALPHA, 1.0f), "%s", tr("No active task", "Nessun task attivo"));
    }

    // Top-right buttons: push to the right using SameLine spacing
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - TOP_RIGHT_BTN_W * TOP_RIGHT_BTN_N - TOP_RIGHT_SPACING * TOP_RIGHT_BTN_N);

    if (ImGui::Button(settings.language == 0 ? "IT" : "EN", ImVec2(TOP_RIGHT_BTN_W, 0))) {
        settings.language = settings.language == 0 ? 1 : 0;
        save_state();
    }
    ImGui::SameLine();
    if (ImGui::Button(settings.dark_mode ? "Light" : "Dark", ImVec2(TOP_RIGHT_BTN_W, 0))) {
        settings.dark_mode = !settings.dark_mode;
        if (settings.dark_mode) ImGui::StyleColorsDark();
        else ImGui::StyleColorsLight();
        init();
        save_state();
    }
    ImGui::SameLine();
    if (ImGui::Button(tr("Statistics", "Statistiche"), ImVec2(TOP_RIGHT_BTN_W, 0))) {
        show_stats = !show_stats;
    }

    ImGui::Separator();

    // Task grid
    ImGui::BeginChild("task_grid", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    auto visible = task_mgr.visible_tasks();
    float avail_w = ImGui::GetContentRegionAvail().x;
    float tbtn_w = std::max(MIN_TASK_BTN_W, (avail_w - GRID_PADDING) / GRID_DIV);
    int cols = std::max(1, (int)(avail_w / (tbtn_w + GRID_GAP)));

    for (size_t i = 0; i < visible.size(); i++) {
        if (i % cols != 0) ImGui::SameLine();
        render_task_button(*visible[i], tbtn_w);
    }

    if (visible.size() % cols != 0) ImGui::SameLine();
    if (ImGui::Button("+", ImVec2(tbtn_w, ADD_BTN_H))) {
        show_add_dialog = true;
        new_task_name[0] = '\0';
    }

    ImGui::EndChild();
    ImGui::End();
}

void AppGUI::render_task_button(Task& task, float width) {
    bool is_active = (task.id == interval_mgr.active_task_id);
    int64_t elapsed = interval_mgr.elapsed_today(task.id);
    // elapsed_today already includes active interval (uses unix_now() for end_ts==0)
    std::string es = format_duration(elapsed);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(task.color_r, task.color_g, task.color_b, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(task.color_r, task.color_g, task.color_b, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(task.color_r, task.color_g, task.color_b, 0.7f));

    if (is_active) {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 1));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, ACTIVE_BORDER);
    }

    std::string label = task.name + "\n" + es;
    if (ImGui::Button(label.c_str(), ImVec2(width, TASK_BTN_H))) {
        interval_mgr.toggle_task(task.id, unix_now());
        save_state();
    }

    if (is_active) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }
    ImGui::PopStyleColor(3);

    // Drag & drop
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload("TASK_ORDER", &task.id, sizeof(uint64_t));
        ImGui::Text("Reorder: %s", task.name.c_str());
        ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TASK_ORDER")) {
            uint64_t src_id = *(const uint64_t*)payload->Data;
            if (src_id != task.id) {
                Task* src = task_mgr.find(src_id);
                if (src) {
                    int so = src->order;
                    src->order = task.order;
                    task.order = so;
                    task_mgr.reassign_orders();
                    save_state();
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void AppGUI::render_add_task_dialog() {
    if (!show_add_dialog) return;

    ImGui::OpenPopup(tr("Add Task", "Aggiungi Task"));
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(tr("Add Task", "Aggiungi Task"), &show_add_dialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", tr("Task Name:", "Nome Task:"));
        ImGui::InputText("##name", new_task_name, NEW_TASK_BUF);

        if (ImGui::Button(tr("Create", "Crea"), ImVec2(DIALOG_BTN_W, 0))) {
            if (new_task_name[0] != '\0') {
                task_mgr.add_task(std::string(new_task_name));
                save_state();
                show_add_dialog = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(tr("Cancel", "Annulla"), ImVec2(DIALOG_BTN_W, 0))) {
            show_add_dialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
