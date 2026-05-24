#include <SDL.h>
#include <SDL_ttf.h>
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "gui.h"
#include "stats.h"
#include "utils.h"
#include <cstdio>

// Window dimensions
static constexpr int    MAIN_WIN_W       = 800;
static constexpr int    MAIN_WIN_H       = 600;
static constexpr int    STATS_WIN_W      = 950;
static constexpr int    STATS_WIN_H      = 700;
static constexpr int    MIN_WIN_SIZE     = 100;

// Font
static constexpr float  FONT_SIZE        = 16.0f;

// Background colors
static constexpr float  BG_DARK          = 0.12f;
static constexpr float  BG_LIGHT         = 0.94f;
static constexpr float  BG_ALPHA         = 1.00f;

// Color conversion
static constexpr int    COLOR_MULT       = 255;

// Frame rate
static constexpr int    FRAME_DELAY_MS   = 16;

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("Error: SDL_Init: %s\n", SDL_GetError());
        return -1;
    }
    if (TTF_Init() != 0) {
        printf("Error: TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return -1;
    }

    // ---- Main window ----
    SDL_Window* win = SDL_CreateWindow("MopTracker - Time Tracker",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        MAIN_WIN_W, MAIN_WIN_H, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!win) { printf("Error: SDL_CreateWindow\n"); TTF_Quit(); SDL_Quit(); return -1; }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!ren) { printf("Error: SDL_CreateRenderer\n"); SDL_DestroyWindow(win); TTF_Quit(); SDL_Quit(); return -1; }

    IMGUI_CHECKVERSION();
    ImGuiContext* main_ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(main_ctx);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    ImGui_ImplSDL2_InitForSDLRenderer(win, ren);
    ImGui_ImplSDLRenderer2_Init(ren);

    io.Fonts->AddFontDefault();
    const char* fps[] = {
#ifdef _WIN32
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
#else
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
#endif
        nullptr
    };
    for (int i = 0; fps[i]; i++) {
        ImFont* f = io.Fonts->AddFontFromFileTTF(fps[i], FONT_SIZE);
        if (f) break;
    }

    // ---- Stats window (separate OS window on Windows) ----
#ifdef _WIN32
    SDL_Window* stats_win = SDL_CreateWindow("MopTracker - Statistics",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        STATS_WIN_W, STATS_WIN_H, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN | SDL_WINDOW_UTILITY);
    if (!stats_win) stats_win = nullptr;
    SDL_Renderer* stats_ren = nullptr;
    ImGuiContext* stats_ctx = nullptr;
    if (stats_win) {
        stats_ren = SDL_CreateRenderer(stats_win, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
        if (!stats_ren) { SDL_DestroyWindow(stats_win); stats_win = nullptr; }
    }
    if (stats_win) {
        stats_ctx = ImGui::CreateContext();
        ImGui::SetCurrentContext(stats_ctx);
        ImGuiIO& stats_io = ImGui::GetIO();
        stats_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        stats_io.IniFilename = nullptr;
        stats_io.Fonts->AddFontDefault();
        for (int i = 0; fps[i]; i++) {
            ImFont* f = stats_io.Fonts->AddFontFromFileTTF(fps[i], FONT_SIZE);
            if (f) break;
        }
        ImGui_ImplSDL2_InitForSDLRenderer(stats_win, stats_ren);
        ImGui_ImplSDLRenderer2_Init(stats_ren);
        ImGui::SetCurrentContext(main_ctx);
    }
#endif

    AppGUI app;
    app.load_state();

    if (app.settings.window_w > MIN_WIN_SIZE && app.settings.window_h > MIN_WIN_SIZE) {
        SDL_SetWindowPosition(win, app.settings.window_x, app.settings.window_y);
        SDL_SetWindowSize(win, app.settings.window_w, app.settings.window_h);
    }

    bool running = true;
    SDL_Event ev;
    while (running) {
        while (SDL_PollEvent(&ev)) {
#ifdef _WIN32
            if (stats_win) {
                SDL_Window* target = nullptr;
                if (ev.type == SDL_WINDOWEVENT) {
                    target = SDL_GetWindowFromID(ev.window.windowID);
                } else if (ev.type == SDL_MOUSEMOTION) {
                    target = SDL_GetWindowFromID(ev.motion.windowID);
                } else if (ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_MOUSEBUTTONUP) {
                    target = SDL_GetWindowFromID(ev.button.windowID);
                } else if (ev.type == SDL_MOUSEWHEEL) {
                    target = SDL_GetWindowFromID(ev.wheel.windowID);
                } else if (ev.type == SDL_TEXTINPUT || ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {
                    target = SDL_GetKeyboardFocus();
                }

                if (target == stats_win) {
                    ImGui::SetCurrentContext(stats_ctx);
                    ImGui_ImplSDL2_ProcessEvent(&ev);
                    if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_CLOSE) {
                        app.show_stats = false;
                        SDL_HideWindow(stats_win);
                    }
                    ImGui::SetCurrentContext(main_ctx);
                    continue;
                }
            }
#endif
            ImGui_ImplSDL2_ProcessEvent(&ev);
            if (ev.type == SDL_QUIT || (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_CLOSE))
                running = false;
        }

        // ---- Render main window ----
        ImGui::SetCurrentContext(main_ctx);
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        SDL_GetWindowPosition(win, &app.settings.window_x, &app.settings.window_y);
        SDL_GetWindowSize(win, &app.settings.window_w, &app.settings.window_h);

        app.render();

        ImGui::Render();

        ImVec4 clr = app.settings.dark_mode ?
            ImVec4(BG_DARK, BG_DARK, BG_DARK, BG_ALPHA) : ImVec4(BG_LIGHT, BG_LIGHT, BG_LIGHT, BG_ALPHA);
        SDL_SetRenderDrawColor(ren, (Uint8)(clr.x*COLOR_MULT), (Uint8)(clr.y*COLOR_MULT), (Uint8)(clr.z*COLOR_MULT), (Uint8)(clr.w*COLOR_MULT));
        SDL_RenderClear(ren);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), ren);
        SDL_RenderPresent(ren);

        // ---- Render stats OS window ----
#ifdef _WIN32
        if (stats_win) {
            static int stats_last_lang = -1;
            if (app.settings.language != stats_last_lang) {
                stats_last_lang = app.settings.language;
                SDL_SetWindowTitle(stats_win, app.settings.language == 1 ? "MopTracker - Statistiche" : "MopTracker - Statistics");
            }
            bool shown = SDL_GetWindowFlags(stats_win) & SDL_WINDOW_SHOWN;
            if (app.show_stats && !shown) {
                SDL_ShowWindow(stats_win);
            } else if (!app.show_stats && shown) {
                SDL_HideWindow(stats_win);
            }

            if (app.show_stats) {
                ImGui::SetCurrentContext(stats_ctx);

                if (app.settings.dark_mode)
                    ImGui::StyleColorsDark();
                else
                    ImGui::StyleColorsLight();

                ImGui_ImplSDLRenderer2_NewFrame();
                ImGui_ImplSDL2_NewFrame();
                ImGui::NewFrame();

                ImVec2 ds = ImGui::GetIO().DisplaySize;
                if (ds.x > 0 && ds.y > 0) {
                    ImGui::SetNextWindowPos(ImVec2(0, 0));
                    ImGui::SetNextWindowSize(ds);
                    ImGui::Begin("##stats", nullptr,
                        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoSavedSettings);
                    render_stats_content(&app);
                    ImGui::End();
                }

                ImGui::Render();
                ImVec4 sclr = app.settings.dark_mode ?
                    ImVec4(BG_DARK, BG_DARK, BG_DARK, BG_ALPHA) : ImVec4(BG_LIGHT, BG_LIGHT, BG_LIGHT, BG_ALPHA);
                SDL_SetRenderDrawColor(stats_ren, (Uint8)(sclr.x*COLOR_MULT), (Uint8)(sclr.y*COLOR_MULT), (Uint8)(sclr.z*COLOR_MULT), (Uint8)(sclr.w*COLOR_MULT));
                SDL_RenderClear(stats_ren);
                if (ds.x > 0 && ds.y > 0)
                    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), stats_ren);
                SDL_RenderPresent(stats_ren);
                ImGui::SetCurrentContext(main_ctx);
            }
        }
#endif

        SDL_Delay(FRAME_DELAY_MS);
    }

    app.shutdown();

    // Cleanup
#ifdef _WIN32
    if (stats_ctx) {
        ImGui::SetCurrentContext(stats_ctx);
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext(stats_ctx);
    }
    if (stats_ren) SDL_DestroyRenderer(stats_ren);
    if (stats_win) SDL_DestroyWindow(stats_win);
    ImGui::SetCurrentContext(main_ctx);
#endif

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
