#pragma once
#include <SkrImGui/imgui_window_backend.hpp>
#include <SkrContainers/string.hpp>
#include <SkrContainers/optional.hpp>
#include <SkrBase/math.h>
#include <SkrCore/dirty.hpp>
#include <SkrCore/memory/rc.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace skr
{
struct ImGuiRendererBackend;

struct SKR_IMGUI_API ImGuiBackend {
    // ctor & dtor
    ImGuiBackend();
    ~ImGuiBackend();

    // imgui context
    void apply_context();
    void create(
        const ImGuiWindowCreateInfo&   main_wnd_create_info,
        RCUnique<ImGuiRendererBackend> backend
    );
    void destroy();

    // frame
    void pump_message();
    void begin_frame();
    void end_frame();
    void acquire_next_frame();
    void render();

    // imgui functional
    void enable_nav(bool enable = true);
    void enable_docking(bool enable = true);
    void enable_multi_viewport(bool enable = true);
    void enable_ini_file(bool enable = true);
    void enable_log_file(bool enable = true);
    void enable_transparent_docking(bool enable = true);
    void enable_always_tab_bar(bool enable = true);
    void enable_move_window_by_blank_area(bool enable = true);

    // getter
    inline bool                      is_created() const { return _context != nullptr; }
    inline const Trigger&            want_exit() const { return _want_exit; }
    inline ImGuiContext*             context() const { return _context; }
    inline const ImGuiWindowBackend& main_window() const { return _main_window; }
    inline ImGuiWindowBackend&       main_window() { return _main_window; }

private:
    // context & main window
    ImGuiContext*      _context     = nullptr;
    ImGuiWindowBackend _main_window = {};

    // render backend
    RCUnique<ImGuiRendererBackend> _renderer_backend = nullptr;

    // dirty & trigger
    Trigger _pixel_size_changed    = {};
    Trigger _content_scale_changed = {};
    Trigger _want_resize           = {};
    Trigger _want_exit             = {};
};
} // namespace skr