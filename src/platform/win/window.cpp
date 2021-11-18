#include "platform/win/window.h"

#include <stdexcept>
#include <system_error>

#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

namespace
{
// we cannot just use WS_POPUP style
// WS_THICKFRAME: without this the window cannot be resized and so aero snap, de-maximizing and minimizing won't work
// WS_SYSMENU: enables the context menu with the move, close, maximize, minize... commands (shift + right-click on the task bar item)
// WS_CAPTION: enables aero minimize animation/transition
// WS_MAXIMIZEBOX, WS_MINIMIZEBOX: enable minimize/maximize
enum class Style : DWORD
{
    windowed = WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
    aero_borderless = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
    basic_borderless = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX
};

auto maximized(HWND hwnd) -> bool
{
    WINDOWPLACEMENT placement;
    if (!::GetWindowPlacement(hwnd, &placement))
    {
        return false;
    }

    return placement.showCmd == SW_MAXIMIZE;
}

/* Adjust client rect to not spill over monitor edges when maximized.
 * rect(in/out): in: proposed window rect, out: calculated client rect
 * Does nothing if the window is not maximized.
 */
auto adjust_maximized_client_rect(HWND window, RECT& rect) -> void
{
    if (!maximized(window))
    {
        return;
    }

    auto monitor = ::MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
    if (!monitor)
    {
        return;
    }

    MONITORINFO monitor_info{};
    monitor_info.cbSize = sizeof(monitor_info);
    if (!::GetMonitorInfoW(monitor, &monitor_info))
    {
        return;
    }

    // when maximized, make the client area fill just the monitor (without task bar) rect,
    // not the whole window rect which extends beyond the monitor.
    rect = monitor_info.rcWork;
}

auto last_error(const std::string& message) -> std::system_error
{
    return std::system_error(
        std::error_code(::GetLastError(), std::system_category()),
        message);
}

auto window_class(WNDPROC wndproc) -> const wchar_t*
{
    static const wchar_t* window_class_name = [&] {
        WNDCLASSEXW wcx{};
        wcx.cbSize = sizeof(wcx);
        wcx.style = CS_HREDRAW | CS_VREDRAW;
        wcx.hInstance = nullptr;
        wcx.lpfnWndProc = wndproc;
        wcx.lpszClassName = L"BorderlessWindowClass";
        wcx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wcx.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
        const ATOM result = ::RegisterClassExW(&wcx);
        if (!result)
        {
            throw last_error("failed to register window class");
        }
        return wcx.lpszClassName;
    }();
    return window_class_name;
}

auto composition_enabled() -> bool
{
    BOOL composition_enabled = FALSE;
    bool success = ::DwmIsCompositionEnabled(&composition_enabled) == S_OK;
    return composition_enabled && success;
}

auto select_borderless_style() -> Style
{
    return composition_enabled() ? Style::aero_borderless : Style::basic_borderless;
}

auto set_shadow(HWND handle, bool enabled) -> void
{
    if (composition_enabled())
    {
        static const MARGINS shadow_state[2]{{0, 0, 0, 0}, {1, 1, 1, 1}};
        ::DwmExtendFrameIntoClientArea(handle, &shadow_state[enabled]);
    }
}

auto create_window(WNDPROC wndproc, void* userdata) -> unique_handle
{
    auto handle = CreateWindowExW(
        0, window_class(wndproc), L"Borderless Window",
        static_cast<DWORD>(Style::aero_borderless), CW_USEDEFAULT, CW_USEDEFAULT,
        1280, 720, nullptr, nullptr, nullptr, userdata);
    if (!handle)
    {
        throw last_error("failed to create window");
    }
    return unique_handle{handle};
}
} // namespace

BorderlessWindow::BorderlessWindow(
    bool borderless, bool resizable, bool dragable, bool with_shadow)
    : borderless(borderless)
    , resizable(resizable)
    , dragable(dragable)
    , with_shadow(with_shadow)
    , handle{create_window(&BorderlessWindow::WndProc, this)}
{
    set_borderless(borderless);
    set_borderless_shadow(with_shadow);
    ::ShowWindow(handle.get(), SW_SHOW);
}

void BorderlessWindow::set_borderless(bool enabled)
{
    Style new_style = (enabled) ? select_borderless_style() : Style::windowed;
    Style old_style = static_cast<Style>(::GetWindowLongPtrW(handle.get(), GWL_STYLE));

    if (new_style != old_style)
    {
        borderless = enabled;

        ::SetWindowLongPtrW(handle.get(), GWL_STYLE, static_cast<LONG>(new_style));

        // when switching between borderless and windowed, restore appropriate shadow state
        set_shadow(handle.get(), with_shadow && (new_style != Style::windowed));

        // redraw frame
        ::SetWindowPos(handle.get(), nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
        ::ShowWindow(handle.get(), SW_SHOW);
    }
}

void BorderlessWindow::set_borderless_shadow(bool enabled)
{
    if (borderless)
    {
        with_shadow = enabled;
        set_shadow(handle.get(), enabled);
    }
}

auto CALLBACK BorderlessWindow::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept -> LRESULT
{
    if (msg == WM_NCCREATE)
    {
        auto userdata = reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams;
        // store window instance pointer in window user data
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userdata));
    }
    if (auto window_ptr = reinterpret_cast<BorderlessWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA)))
    {
        auto& window = *window_ptr;

        switch (msg)
        {
            case WM_NCCALCSIZE: {
                if (wparam == TRUE && window.borderless)
                {
                    auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);
                    adjust_maximized_client_rect(hwnd, params.rgrc[0]);
                    return 0;
                }
                break;
            }
            case WM_NCHITTEST: {
                // When we have no border or title bar, we need to perform our
                // own hit testing to allow resizing and moving.
                if (window.borderless)
                {
                    return window.hit_test(POINT{
                        GET_X_LPARAM(lparam),
                        GET_Y_LPARAM(lparam)});
                }
                break;
            }
            case WM_NCACTIVATE: {
                if (!composition_enabled())
                {
                    // Prevents window frame reappearing on window activation
                    // in "basic" theme, where no aero shadow is present.
                    return 1;
                }
                break;
            }

            case WM_CLOSE: {
                ::DestroyWindow(hwnd);
                return 0;
            }

            case WM_DESTROY: {
                PostQuitMessage(0);
                return 0;
            }

            case WM_KEYDOWN:
            case WM_SYSKEYDOWN: {
                switch (wparam)
                {
                    case VK_F8: {
                        window.dragable = !window.dragable;
                        return 0;
                    }
                    case VK_F9: {
                        window.resizable = !window.resizable;
                        return 0;
                    }
                    case VK_F10: {
                        window.set_borderless(!window.borderless);
                        return 0;
                    }
                    case VK_F11: {
                        window.set_borderless_shadow(!window.with_shadow);
                        return 0;
                    }
                }
                break;
            }
        }
    }

    return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}

auto BorderlessWindow::hit_test(POINT cursor) const -> LRESULT
{
    // identify borders and corners to allow resizing the window.
    // Note: On Windows 10, windows behave differently and
    // allow resizing outside the visible window frame.
    // This implementation does not replicate that behavior.
    const POINT border{
        ::GetSystemMetrics(SM_CXFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER),
        ::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER)};
    RECT window;
    if (!::GetWindowRect(handle.get(), &window))
    {
        return HTNOWHERE;
    }

    const auto drag = dragable ? HTCAPTION : HTCLIENT;

    enum region_mask
    {
        client = 0b0000,
        left = 0b0001,
        right = 0b0010,
        top = 0b0100,
        bottom = 0b1000,
    };

    const auto result =
        left * (cursor.x < (window.left + border.x)) |
        right * (cursor.x >= (window.right - border.x)) |
        top * (cursor.y < (window.top + border.y)) |
        bottom * (cursor.y >= (window.bottom - border.y));

    switch (result)
    {
        case left:
            return resizable ? HTLEFT : drag;
        case right:
            return resizable ? HTRIGHT : drag;
        case top:
            return resizable ? HTTOP : drag;
        case bottom:
            return resizable ? HTBOTTOM : drag;
        case top | left:
            return resizable ? HTTOPLEFT : drag;
        case top | right:
            return resizable ? HTTOPRIGHT : drag;
        case bottom | left:
            return resizable ? HTBOTTOMLEFT : drag;
        case bottom | right:
            return resizable ? HTBOTTOMRIGHT : drag;
        case client:
            return drag;
        default:
            return HTNOWHERE;
    }
}
