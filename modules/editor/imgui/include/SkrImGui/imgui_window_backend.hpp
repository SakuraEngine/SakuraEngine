#pragma once
#include <SkrContainers/string.hpp>
#include <SkrContainers/optional.hpp>
#include <SkrBase/math.h>

namespace skr
{
struct ImGuiWindowBackend;

struct ImGuiWindowCreateInfo {
    String              title         = {};
    Optional<uint2>     pos           = {}; // default means centered
    uint2               size          = { 1280, 720 };
    ImGuiWindowBackend* popup_parent  = nullptr; // if setted, means create a popup window
    bool                is_topmost    = false;
    bool                is_tooltip    = false;
    bool                is_borderless = false;
    bool                is_resizable  = true;
};

struct SKR_IMGUI_API ImGuiMonitorBackend {
    ImGuiMonitorBackend(uint64_t id);

    // monitor count
    static uint64_t            monitor_count();
    static void                each_monitor(FunctionRef<void(const ImGuiMonitorBackend&)> func);
    static ImGuiMonitorBackend monitor_at(uint64_t id);

    // getter
    uint64_t id() const;
    String   name() const;
    int2     pos() const;
    int2     size() const;
    float    pixel_density() const; // pixel_size / logical_size
    float    display_scale() const; // display scale setted by user

private:
    uint64_t _id = 0;
};

struct SKR_IMGUI_API ImGuiWindowBackend {
    // ctor & dtor
    ImGuiWindowBackend();
    ~ImGuiWindowBackend();

    // create & destroy
    void create(const ImGuiWindowCreateInfo& create_info = {});
    void destroy();
    bool is_valid() const;

    // getter
    String              title() const;
    uint2               size() const;
    uint2               pos() const;
    uint2               size_client() const;
    uint2               pos_client() const;
    uint2               size_client_pixel() const;
    bool                is_hidden() const;
    bool                is_minimized() const;
    bool                is_keyboard_focused() const;
    bool                is_mouse_focused() const;
    bool                is_fullscreen() const;
    bool                is_resizable() const;
    bool                is_borderless() const;
    bool                is_topmost() const;
    bool                is_popup() const;
    bool                is_tooltip() const;
    ImGuiMonitorBackend monitor() const;

    // DPI
    float pixel_density() const; // pixel_size / logical_size
    float display_scale() const; // display scale setted by user

    // setter
    void set_title(skr::StringView title);
    void set_client_size(uint2 size);
    void set_pos(uint2 pos);
    void set_fullscreen(bool true_fullscreen);
    void set_resizable(bool resizable);
    void set_borderless(bool borderless);

    // special pos
    void set_pos_centered();
    void set_pos_centered(ImGuiMonitorBackend monitor);

    // operators
    void show();
    void hide();
    void raise();
    void maximize();
    void minimize();
    void restore();

    // native
    void* native_handle() const;
    void* payload() const;

private:
    void* _payload = nullptr;
};
} // namespace skr