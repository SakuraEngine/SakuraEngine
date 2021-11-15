#pragma once
#include "platform/configure.h"
#include "EASTL/unique_ptr.h"

#ifndef WIN_MEAN_AND_LEAN
#define WIN_MEAN_AND_LEAN
#endif
#include "windows.h"

struct hwnd_deleter {
    using pointer = HWND;
    auto operator()(HWND handle) const -> void {
        ::DestroyWindow(handle);
    }
};

using unique_handle = eastl::unique_ptr<HWND, hwnd_deleter>;

// based on https://github.com/melak47/BorderlessWindow
class BorderlessWindow 
{
public:
    RUNTIME_API BorderlessWindow(bool borderless = true, bool resizable = true,
        bool dragable = true, bool with_shadow = true);
    RUNTIME_API auto set_borderless(bool enabled) -> void;
    RUNTIME_API auto set_borderless_shadow(bool enabled) -> void;
    inline HWND get_hwnd() {return handle.get();}
private:
    static auto CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept -> LRESULT;
    auto hit_test(POINT cursor) const -> LRESULT;

    bool borderless        = true; // is the window currently borderless
    bool resizable = true; // should the window allow resizing by dragging the borders while borderless
    bool dragable   = true; // should the window allow moving my dragging the client area
    bool with_shadow = true; // should the window display a native aero shadow while borderless

    unique_handle handle;
};