#include <SkrImGui/imgui_window_backend.hpp>
#include <SDL3/SDL.h>

namespace skr
{
ImGuiMonitorBackend::ImGuiMonitorBackend(uint64_t id)
    : _id(id)
{
}

// monitor count
uint64_t ImGuiMonitorBackend::monitor_count()
{
    int count;
    SDL_GetDisplays(&count);
    return count;
}
void ImGuiMonitorBackend::each_monitor(FunctionRef<void(const ImGuiMonitorBackend&)> func)
{
    int            count;
    SDL_DisplayID* ids = SDL_GetDisplays(&count);
    for (int i = 0; i < count; i++)
    {
        ImGuiMonitorBackend monitor(ids[i]);
        func(monitor);
    }
}
ImGuiMonitorBackend ImGuiMonitorBackend::monitor_at(uint64_t id)
{
    return ImGuiMonitorBackend(id);
}

// getter
uint64_t ImGuiMonitorBackend::id() const
{
    return _id;
}
String ImGuiMonitorBackend::name() const
{
    return String::From(SDL_GetDisplayName(_id));
}
int2 ImGuiMonitorBackend::pos() const
{
    SDL_Rect rect;
    SDL_GetDisplayBounds(_id, &rect);
    return int2{ rect.x, rect.y };
}
int2 ImGuiMonitorBackend::size() const
{
    SDL_Rect rect;
    SDL_GetDisplayBounds(_id, &rect);
    return int2{ rect.w, rect.h };
}
float ImGuiMonitorBackend::pixel_density() const
{
    const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(_id);
    return mode->pixel_density;
}

// ctor & dtor
ImGuiWindowBackend::ImGuiWindowBackend()
{
}
ImGuiWindowBackend::~ImGuiWindowBackend()
{
    destroy();
}

// create & destroy
void ImGuiWindowBackend::create(const ImGuiWindowCreateInfo& create_info)
{
    SKR_ASSERT(_payload == nullptr);

    // combine flags
    SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (create_info.is_topmost) { flags |= SDL_WINDOW_ALWAYS_ON_TOP; }
    if (create_info.is_tooltip) { flags |= SDL_WINDOW_TOOLTIP; }
    if (create_info.is_borderless) { flags |= SDL_WINDOW_BORDERLESS; }
    if (create_info.is_resizable) { flags |= SDL_WINDOW_RESIZABLE; }

    // create window
    if (create_info.popup_parent)
    {
        SDL_Window* parent = reinterpret_cast<SDL_Window*>(create_info.popup_parent->_payload);
        _payload           = SDL_CreatePopupWindow(
            parent,
            create_info.pos.has_value() ? create_info.pos->x : SDL_WINDOWPOS_CENTERED,
            create_info.pos.has_value() ? create_info.pos->y : SDL_WINDOWPOS_CENTERED,
            create_info.size.x,
            create_info.size.y,
            flags
        );
    }
    else
    {
        SDL_Window* wnd = SDL_CreateWindow(
            create_info.title.c_str_raw(),
            create_info.size.x,
            create_info.size.y,
            flags
        );

        if (create_info.pos.has_value())
        {
            SDL_SetWindowPosition(wnd, create_info.pos->x, create_info.pos->y);
        }
        else
        {
            SDL_SetWindowPosition(wnd, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }

        _payload = wnd;
    }
}
void ImGuiWindowBackend::destroy()
{
    if (_payload)
    {
        SDL_DestroyWindow(reinterpret_cast<SDL_Window*>(_payload));
        _payload = nullptr;
    }
}
bool ImGuiWindowBackend::is_valid() const
{
    return _payload != nullptr;
}

// getter
String ImGuiWindowBackend::title() const
{
    auto title = SDL_GetWindowTitle(reinterpret_cast<SDL_Window*>(_payload));
    return String::From(title);
}
uint2 ImGuiWindowBackend::size() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);

    int t, l, b, r;
    SDL_GetWindowBordersSize(wnd, &t, &l, &b, &r);
    int w, h;
    SDL_GetWindowSize(wnd, &w, &h);
    return {
        static_cast<uint32_t>(w + l + r),
        static_cast<uint32_t>(h + t + b)
    };
}
uint2 ImGuiWindowBackend::pos() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);

    int x, y;
    SDL_GetWindowPosition(wnd, &x, &y);
    return {
        static_cast<uint32_t>(x),
        static_cast<uint32_t>(y)
    };
}
uint2 ImGuiWindowBackend::size_client() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);

    int w, h;
    SDL_GetWindowSize(wnd, &w, &h);
    return {
        static_cast<uint32_t>(w),
        static_cast<uint32_t>(h)
    };
}
uint2 ImGuiWindowBackend::pos_client() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);

    int x, y;
    SDL_GetWindowPosition(wnd, &x, &y);
    int t, l, b, r;
    SDL_GetWindowBordersSize(wnd, &t, &l, &b, &r);
    return {
        static_cast<uint32_t>(x - l),
        static_cast<uint32_t>(y - t)
    };
}
uint2 ImGuiWindowBackend::size_client_pixel() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);

    int w, h;
    SDL_GetWindowSizeInPixels(wnd, &w, &h);
    return {
        static_cast<uint32_t>(w),
        static_cast<uint32_t>(h)
    };
}
bool ImGuiWindowBackend::is_hidden() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    return SDL_GetWindowFlags(wnd) & SDL_WINDOW_HIDDEN;
}
bool ImGuiWindowBackend::is_minimized() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    return SDL_GetWindowFlags(wnd) & SDL_WINDOW_MINIMIZED;
}
bool ImGuiWindowBackend::is_keyboard_focused() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    return SDL_GetWindowFlags(wnd) & SDL_WINDOW_INPUT_FOCUS;
}
bool ImGuiWindowBackend::is_mouse_focused() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    return SDL_GetWindowFlags(wnd) & SDL_WINDOW_MOUSE_FOCUS;
}
bool ImGuiWindowBackend::is_fullscreen() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    return SDL_GetWindowFlags(wnd) & SDL_WINDOW_FULLSCREEN;
}
bool ImGuiWindowBackend::is_resizable() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    return SDL_GetWindowFlags(wnd) & SDL_WINDOW_RESIZABLE;
}
bool ImGuiWindowBackend::is_borderless() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    return SDL_GetWindowFlags(wnd) & SDL_WINDOW_BORDERLESS;
}
bool ImGuiWindowBackend::is_topmost() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    return SDL_GetWindowFlags(wnd) & SDL_WINDOW_ALWAYS_ON_TOP;
}
bool ImGuiWindowBackend::is_popup() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    return SDL_GetWindowFlags(wnd) & SDL_WINDOW_POPUP_MENU;
}
bool ImGuiWindowBackend::is_tooltip() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    return SDL_GetWindowFlags(wnd) & SDL_WINDOW_TOOLTIP;
}
ImGuiMonitorBackend ImGuiWindowBackend::monitor() const
{
    auto*         wnd = reinterpret_cast<SDL_Window*>(_payload);
    SDL_DisplayID id  = SDL_GetDisplayForWindow(wnd);
    return ImGuiMonitorBackend(id);
}

// DPI
float ImGuiWindowBackend::pixel_density() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    return SDL_GetWindowPixelDensity(wnd);
}
float ImGuiWindowBackend::display_scale() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    return SDL_GetWindowDisplayScale(wnd);
}

// setter
void ImGuiWindowBackend::set_title(skr::StringView title)
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    SDL_SetWindowTitle(wnd, title.data_raw());
}
void ImGuiWindowBackend::set_client_size(uint2 size)
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    SDL_SetWindowSize(wnd, static_cast<int>(size.x), static_cast<int>(size.y));
}
void ImGuiWindowBackend::set_pos(uint2 pos)
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    SDL_SetWindowPosition(wnd, static_cast<int>(pos.x), static_cast<int>(pos.y));
}
void ImGuiWindowBackend::set_fullscreen(bool true_fullscreen)
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    if (true_fullscreen)
    {
        SDL_SetWindowFullscreen(wnd, true);
    }
    else
    {
        SDL_SetWindowFullscreen(wnd, false);
    }
}
void ImGuiWindowBackend::set_resizable(bool resizable)
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    SDL_SetWindowResizable(wnd, resizable);
}
void ImGuiWindowBackend::set_borderless(bool borderless)
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    SDL_SetWindowBordered(wnd, !borderless);
}

// special pos
void ImGuiWindowBackend::set_pos_centered()
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    SDL_SetWindowPosition(wnd, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}
void ImGuiWindowBackend::set_pos_centered(ImGuiMonitorBackend monitor)
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    SDL_SetWindowPosition(
        wnd,
        SDL_WINDOWPOS_CENTERED_DISPLAY(monitor.id()),
        SDL_WINDOWPOS_CENTERED_DISPLAY(monitor.id())
    );
}

// operators
void ImGuiWindowBackend::show()
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    SDL_ShowWindow(wnd);
}
void ImGuiWindowBackend::hide()
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    SDL_HideWindow(wnd);
}
void ImGuiWindowBackend::raise()
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    SDL_RaiseWindow(wnd);
}
void ImGuiWindowBackend::maximize()
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    SDL_MaximizeWindow(wnd);
}
void ImGuiWindowBackend::minimize()
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    SDL_MinimizeWindow(wnd);
}
void ImGuiWindowBackend::restore()
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);
    SDL_RestoreWindow(wnd);
}

// native
void* ImGuiWindowBackend::native_handle() const
{
    auto* wnd = reinterpret_cast<SDL_Window*>(_payload);

    auto prop_id = SDL_GetWindowProperties(wnd);
#if SKR_PLAT_WINDOWS
    return SDL_GetPointerProperty(prop_id, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif SKR_PLAT_MACOSX
    return SDL_GetPointerProperty(prop_id, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#else
    return nullptr;
#endif
}
void* ImGuiWindowBackend::payload() const
{
    return _payload;
}

} // namespace skr