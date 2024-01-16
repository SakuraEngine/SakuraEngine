#pragma once
#include "SkrRT/platform/system.h"
#include "SkrContainers/vector.hpp"
#include "SkrContainers/map.hpp"

namespace skr
{

struct SKR_RUNTIME_API SystemMessageHandlerProxy : public ISystemMessageHandler {
    template <typename T>
    using Handler = std::pair<T, void*>;
    template <typename T>
    using RIDMap = skr::Map<int64_t, T>;

    void on_window_resized(SWindowHandle window, int32_t w, int32_t h) SKR_NOEXCEPT
    {
        for (auto& handler : message_handlers)
        {
            handler->on_window_resized(window, w, h);
        }
        for (auto& handler : window_resize_handlers)
        {
            handler.value.first(window, w, h, handler.value.second);
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
            handler.value.first(window, handler.value.second);
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
            handler.value.first(wheelX, wheelY, handler.value.second);
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
            handler.value.first(button, x, y, handler.value.second);
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
            handler.value.first(button, x, y, handler.value.second);
        }
    }

    void on_key_down(EKeyCode key)
    {
        for (auto& handler : message_handlers)
        {
            handler->on_key_down(key);
        }
        for (auto& handler : key_down_handlers)
        {
            handler.value.first(key, handler.value.second);
        }
    }

    void on_key_up(EKeyCode key)
    {
        for (auto& handler : message_handlers)
        {
            handler->on_key_up(key);
        }
        for (auto& handler : key_up_handlers)
        {
            handler.value.first(key, handler.value.second);
        }
    }

    void on_text_input(const char8_t* text)
    {
        for (auto& handler : message_handlers)
        {
            handler->on_text_input(text);
        }
        for (auto& handler : text_input_handlers)
        {
            handler.value.first(text, handler.value.second);
        }
    }

    inline void add_message_handler(ISystemMessageHandler* handler) SKR_NOEXCEPT
    {
        message_handlers.add(handler);
    }

    inline void remove_message_handler(ISystemMessageHandler* handler) SKR_NOEXCEPT
    {
        message_handlers.remove(handler);
    }

    inline int64_t add_window_resize_handler(SWindowResizeHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        window_resize_handlers.add(rid, { proc, usr_data });
        return rid;
    }

    void remove_window_resize_handler(int64_t rid) SKR_NOEXCEPT
    {
        window_resize_handlers.remove(rid);
    }

    int64_t add_window_close_handler(SWindowCloseHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        window_close_handlers.add(rid, { proc, usr_data });
        return rid;
    }

    void remove_window_close_handler(int64_t rid) SKR_NOEXCEPT
    {
        window_close_handlers.remove(rid);
    }

    int64_t add_window_move_handler(SWindowMoveHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        window_move_handlers.add(rid, { proc, usr_data });
        return rid;
    }

    void remove_window_move_handler(int64_t rid) SKR_NOEXCEPT
    {
        window_move_handlers.remove(rid);
    }

    int64_t add_mouse_wheel_handler(SMouseWheelHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        mouse_wheel_handlers.add(rid, { proc, usr_data });
        return rid;
    }

    void remove_mouse_wheel_handler(int64_t rid) SKR_NOEXCEPT
    {
        mouse_wheel_handlers.remove(rid);
    }

    int64_t add_mouse_button_down_handler(SMouseButtonDownHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        mouse_button_down_handlers.add(rid, { proc, usr_data });
        return rid;
    }

    void remove_mouse_button_down_handler(int64_t rid) SKR_NOEXCEPT
    {
        mouse_button_down_handlers.remove(rid);
    }

    int64_t add_mouse_button_up_handler(SMouseButtonUpHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        mouse_button_up_handlers.add(rid, { proc, usr_data });
        return rid;
    }

    void remove_mouse_button_up_handler(int64_t rid) SKR_NOEXCEPT
    {
        mouse_button_up_handlers.remove(rid);
    }

    int64_t add_key_down_handler(SKeyDownHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        key_down_handlers.add(rid, { proc, usr_data });
        return rid;
    }

    void remove_key_down_handler(int64_t rid) SKR_NOEXCEPT
    {
        key_down_handlers.remove(rid);
    }

    int64_t add_key_up_handler(SKeyUpHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        key_up_handlers.add(rid, { proc, usr_data });
        return rid;
    }

    void remove_key_up_handler(int64_t rid) SKR_NOEXCEPT
    {
        key_up_handlers.remove(rid);
    }

    int64_t add_text_input_handler(STextInputHandlerProc proc, void* usr_data) SKR_NOEXCEPT
    {
        auto rid = next_handler_id++;
        text_input_handlers.add(rid, { proc, usr_data });
        return rid;
    }

    void remove_text_input_handler(int64_t rid) SKR_NOEXCEPT
    {
        text_input_handlers.remove(rid);
    }

    skr::Vector<ISystemMessageHandler*>          message_handlers;
    RIDMap<Handler<SWindowResizeHandlerProc>>    window_resize_handlers;
    RIDMap<Handler<SWindowCloseHandlerProc>>     window_close_handlers;
    RIDMap<Handler<SWindowMoveHandlerProc>>      window_move_handlers;
    RIDMap<Handler<SMouseWheelHandlerProc>>      mouse_wheel_handlers;
    RIDMap<Handler<SMouseButtonDownHandlerProc>> mouse_button_down_handlers;
    RIDMap<Handler<SMouseButtonUpHandlerProc>>   mouse_button_up_handlers;
    RIDMap<Handler<SKeyDownHandlerProc>>         key_down_handlers;
    RIDMap<Handler<SKeyUpHandlerProc>>           key_up_handlers;
    RIDMap<Handler<STextInputHandlerProc>>       text_input_handlers;
    int64_t                                      next_handler_id = 0;
};

struct SystemHandlerBase : public ISystemHandler {
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

    virtual int64_t add_key_down_handler(SKeyDownHandlerProc proc, void* usr_data) SKR_NOEXCEPT final
    {
        return message_handler_proxy.add_key_down_handler(proc, usr_data);
    }

    virtual void remove_key_down_handler(int64_t rid) SKR_NOEXCEPT final
    {
        message_handler_proxy.remove_key_down_handler(rid);
    }

    virtual int64_t add_key_up_handler(SKeyUpHandlerProc proc, void* usr_data) SKR_NOEXCEPT final
    {
        return message_handler_proxy.add_key_up_handler(proc, usr_data);
    }

    virtual void remove_key_up_handler(int64_t rid) SKR_NOEXCEPT final
    {
        message_handler_proxy.remove_key_up_handler(rid);
    }

    virtual int64_t add_text_input_handler(STextInputHandlerProc proc, void* usr_data) SKR_NOEXCEPT final
    {
        return message_handler_proxy.add_text_input_handler(proc, usr_data);
    }

    virtual void remove_text_input_handler(int64_t rid) SKR_NOEXCEPT final
    {
        message_handler_proxy.remove_text_input_handler(rid);
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

} // namespace skr