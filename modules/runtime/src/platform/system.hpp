#pragma once
#include "platform/system.h"
#include "containers/vector.hpp"

#include <EASTL/vector_map.h>

namespace skr {

struct RUNTIME_API SystemMessageHandlerProxy : public ISystemMessageHandler
{
    template <typename T>
    using RIDMap = eastl::vector_map<int64_t, T>;

    void on_window_resized(SWindowHandle window, int32_t w, int32_t h) SKR_NOEXCEPT
    {
        for (auto& handler : message_handlers)
        {
            handler->on_window_resized(window, w, h);
        }
        for (auto& handler : window_resize_handlers)
        {
            handler.second.first(window, w, h, handler.second.second);
        }
    }
    void on_window_closed(SWindowHandle window) SKR_NOEXCEPT
    {
        for (auto& handler : message_handlers)
        {
            handler->on_window_closed(window);
        }
        for (auto& handler : window_close_handlers)
        {
            handler.second.first(window, handler.second.second);
        }
    }

    void on_mouse_wheel(int32_t wheelX, int32_t wheelY)
    {
        for (auto& handler : message_handlers)
        {
            handler->on_mouse_wheel(wheelX, wheelY);
        }
        for (auto& handler : mouse_wheel_handlers)
        {
            handler.second.first(wheelX, wheelY, handler.second.second);
        }
    }

    void on_mouse_button_down(EMouseKey button, int32_t x, int32_t y)
    {
        for (auto& handler : message_handlers)
        {
            handler->on_mouse_button_down(button, x, y);
        }
        for (auto& handler : mouse_button_down_handlers)
        {
            handler.second.first(button, x, y, handler.second.second);
        }
    }

    void on_mouse_button_up(EMouseKey button, int32_t x, int32_t y)
    {
        for (auto& handler : message_handlers)
        {
            handler->on_mouse_button_up(button, x, y);
        }
        for (auto& handler : mouse_button_up_handlers)
        {
            handler.second.first(button, x, y, handler.second.second);
        }
    }

    inline void add_message_handler(ISystemMessageHandler* handler) SKR_NOEXCEPT
    {
        message_handlers.emplace_back(handler);
    }

    inline void remove_message_handler(ISystemMessageHandler* handler) SKR_NOEXCEPT
    {
        message_handlers.erase(
            eastl::find(message_handlers.begin(), message_handlers.end(), handler)
        );
    }

    inline int64_t add_window_resize_handler(SWindowResizeHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        window_resize_handlers[rid] = { proc, usr_data };
        return rid;
    }

    void remove_window_resize_handler(int64_t rid) SKR_NOEXCEPT
    {
        window_resize_handlers.erase(rid);
    }

    int64_t add_window_close_handler(SWindowCloseHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        window_close_handlers[rid] = { proc, usr_data };
        return rid;
    }

    void remove_window_close_handler(int64_t rid) SKR_NOEXCEPT
    {
        window_close_handlers.erase(rid);
    }

    int64_t add_window_move_handler(SWindowMoveHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        window_move_handlers[rid] = { proc, usr_data };
        return rid;
    }

    void remove_window_move_handler(int64_t rid) SKR_NOEXCEPT
    {
        window_move_handlers.erase(rid);
    }

    int64_t add_mouse_wheel_handler(SMouseWheelHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        mouse_wheel_handlers[rid] = { proc, usr_data };
        return rid;
    }

    void remove_mouse_wheel_handler(int64_t rid) SKR_NOEXCEPT
    {
        mouse_wheel_handlers.erase(rid);
    }

    int64_t add_mouse_button_down_handler(SMouseButtonDownHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        mouse_button_down_handlers[rid] = { proc, usr_data };
        return rid;
    }

    void remove_mouse_button_down_handler(int64_t rid) SKR_NOEXCEPT
    {
        mouse_button_down_handlers.erase(rid);
    }

    int64_t add_mouse_button_up_handler(SMouseButtonUpHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        mouse_button_up_handlers[rid] = { proc, usr_data };
        return rid;
    }

    void remove_mouse_button_up_handler(int64_t rid) SKR_NOEXCEPT
    {
        mouse_button_up_handlers.erase(rid);
    }

    skr::vector<ISystemMessageHandler*> message_handlers;
    RIDMap<eastl::pair<SWindowResizeHandlerProc, void*>> window_resize_handlers;
    RIDMap<eastl::pair<SWindowCloseHandlerProc, void*>> window_close_handlers;
    RIDMap<eastl::pair<SWindowMoveHandlerProc, void*>> window_move_handlers;
    RIDMap<eastl::pair<SMouseWheelHandlerProc, void*>> mouse_wheel_handlers;
    RIDMap<eastl::pair<SMouseButtonDownHandlerProc, void*>> mouse_button_down_handlers;
    RIDMap<eastl::pair<SMouseButtonUpHandlerProc, void*>> mouse_button_up_handlers;
    int64_t next_handler_id = 0;
};

struct SystemHandlerBase : public ISystemHandler
{
    virtual void add_message_handler(ISystemMessageHandler* handler) SKR_NOEXCEPT override;
    virtual void remove_message_handler(ISystemMessageHandler* handler) SKR_NOEXCEPT override;

    virtual int64_t add_window_resize_handler(SWindowResizeHandlerProc proc, void* usr_data) SKR_NOEXCEPT final
    {
        return message_handler_proxy.add_window_resize_handler(proc, usr_data);
    }

    virtual void remove_window_resize_handler(int64_t rid) SKR_NOEXCEPT final
    {
        message_handler_proxy.remove_window_resize_handler(rid);
    }

    virtual int64_t add_window_close_handler(SWindowCloseHandlerProc proc, void* usr_data) SKR_NOEXCEPT final
    {
        return message_handler_proxy.add_window_close_handler(proc, usr_data);
    }

    virtual void remove_window_close_handler(int64_t rid) SKR_NOEXCEPT final
    {
        message_handler_proxy.remove_window_close_handler(rid);
    }

    virtual int64_t add_window_move_handler(SWindowMoveHandlerProc proc, void* usr_data) SKR_NOEXCEPT final
    {
        return message_handler_proxy.add_window_move_handler(proc, usr_data);
    }

    virtual void remove_window_move_handler(int64_t rid) SKR_NOEXCEPT final
    {
        message_handler_proxy.remove_window_move_handler(rid);
    }

    virtual int64_t add_mouse_wheel_handler(SMouseWheelHandlerProc proc, void* usr_data) SKR_NOEXCEPT final
    {
        return message_handler_proxy.add_mouse_wheel_handler(proc, usr_data);
    }

    virtual void remove_mouse_wheel_handler(int64_t rid) SKR_NOEXCEPT final
    {
        message_handler_proxy.remove_mouse_wheel_handler(rid);
    }

    virtual int64_t add_mouse_button_down_handler(SMouseButtonDownHandlerProc proc, void* usr_data) SKR_NOEXCEPT final
    {
        return message_handler_proxy.add_mouse_button_down_handler(proc, usr_data);
    }

    virtual void remove_mouse_button_down_handler(int64_t rid) SKR_NOEXCEPT final
    {
        message_handler_proxy.remove_mouse_button_down_handler(rid);
    }

    virtual int64_t add_mouse_button_up_handler(SMouseButtonUpHandlerProc proc, void* usr_data) SKR_NOEXCEPT final
    {
        return message_handler_proxy.add_mouse_button_up_handler(proc, usr_data);
    }

    virtual void remove_mouse_button_up_handler(int64_t rid) SKR_NOEXCEPT final
    {
        message_handler_proxy.remove_mouse_button_up_handler(rid);
    }

    SystemMessageHandlerProxy message_handler_proxy;

};

inline void SystemHandlerBase::add_message_handler(ISystemMessageHandler* handler) SKR_NOEXCEPT
{
    message_handler_proxy.add_message_handler(handler);
}

inline void SystemHandlerBase::remove_message_handler(ISystemMessageHandler* uid) SKR_NOEXCEPT
{
    message_handler_proxy.remove_message_handler(uid);
}

}