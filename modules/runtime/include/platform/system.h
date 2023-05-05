#pragma once
#include "utils/types.h"
#include "platform/window.h"
#include "platform/input.h"

SKR_DECLARE_TYPE_ID_FWD(skr, ISystemHandler, skr_system_handler)
SKR_DECLARE_TYPE_ID_FWD(skr, ISystemMessageHandler, skr_system_message_handler)

typedef void(*SWindowResizeHandlerProc)(SWindowHandle, int32_t w, int32_t h, void* usr_data);
typedef void(*SWindowCloseHandlerProc)(SWindowHandle, void* usr_data);
typedef void(*SWindowMoveHandlerProc)(SWindowHandle, void* usr_data);
typedef void(*SMouseWheelHandlerProc)(int32_t wheelX, int32_t wheelY, void* usr_data);
typedef void(*SMouseButtonDownHandlerProc)(EMouseKey button, int32_t x, int32_t y, void* usr_data);
typedef void(*SMouseButtonUpHandlerProc)(EMouseKey button, int32_t x, int32_t y, void* usr_data);
typedef void(*SKeyDownHandlerProc)(EKeyCode button, void* usr_data);
typedef void(*SKeyUpHandlerProc)(EKeyCode button, void* usr_data);
typedef void(*STextInputHandlerProc)(const char8_t* text, void* usr_data);

#ifdef __cplusplus
namespace skr 
{

struct RUNTIME_API ISystemMessageHandler
{
    virtual ~ISystemMessageHandler() SKR_NOEXCEPT;
    
    virtual void on_window_resized(SWindowHandle, int32_t w, int32_t h) SKR_NOEXCEPT {};
    virtual void on_window_closed(SWindowHandle) SKR_NOEXCEPT {};
    virtual void on_window_moved(SWindowHandle) SKR_NOEXCEPT {};
    virtual void on_mouse_wheel(int32_t wheelX, int32_t wheelY) {};
    virtual void on_mouse_button_down(EMouseKey button, int32_t x, int32_t y) {};
    virtual void on_mouse_button_up(EMouseKey button, int32_t x, int32_t y) {};
    virtual void on_key_down(EKeyCode) {};
    virtual void on_key_up(EKeyCode) {};
    virtual void on_text_input(const char8_t* text) {};
};

struct RUNTIME_API ISystemHandler
{
    virtual ~ISystemHandler() SKR_NOEXCEPT;

    virtual void pump_messages(float delta) SKR_NOEXCEPT = 0;
    virtual void process_messages(float delta) SKR_NOEXCEPT = 0;
    virtual void update_device_states() SKR_NOEXCEPT = 0;
    virtual void* load_handler_proc(const char* name) SKR_NOEXCEPT = 0; 

    virtual void add_message_handler(ISystemMessageHandler* handler) SKR_NOEXCEPT = 0;
    virtual void remove_message_handler(ISystemMessageHandler* handler) SKR_NOEXCEPT = 0;

    virtual int64_t add_window_resize_handler(SWindowResizeHandlerProc, void*) SKR_NOEXCEPT = 0;
    virtual void remove_window_resize_handler(int64_t rid) SKR_NOEXCEPT = 0;
    virtual int64_t add_window_close_handler(SWindowCloseHandlerProc, void*) SKR_NOEXCEPT = 0;
    virtual void remove_window_close_handler(int64_t rid) SKR_NOEXCEPT = 0;
    virtual int64_t add_window_move_handler(SWindowMoveHandlerProc, void*) SKR_NOEXCEPT = 0;
    virtual void remove_window_move_handler(int64_t rid) SKR_NOEXCEPT = 0;
    virtual int64_t add_mouse_wheel_handler(SMouseWheelHandlerProc, void*) SKR_NOEXCEPT = 0;
    virtual void remove_mouse_wheel_handler(int64_t rid) SKR_NOEXCEPT = 0;
    virtual int64_t add_mouse_button_down_handler(SMouseButtonDownHandlerProc, void*) SKR_NOEXCEPT = 0;
    virtual void remove_mouse_button_down_handler(int64_t rid) SKR_NOEXCEPT = 0;
    virtual int64_t add_mouse_button_up_handler(SMouseButtonUpHandlerProc, void*) SKR_NOEXCEPT = 0;
    virtual void remove_mouse_button_up_handler(int64_t rid) SKR_NOEXCEPT = 0;
    virtual int64_t add_key_down_handler(SKeyDownHandlerProc, void*) SKR_NOEXCEPT = 0;
    virtual void remove_key_down_handler(int64_t rid) SKR_NOEXCEPT = 0;
    virtual int64_t add_key_up_handler(SKeyUpHandlerProc, void*) SKR_NOEXCEPT = 0;
    virtual void remove_key_up_handler(int64_t rid) SKR_NOEXCEPT = 0;
    virtual int64_t add_text_input_handler(STextInputHandlerProc, void*) SKR_NOEXCEPT = 0;
    virtual void remove_text_input_handler(int64_t rid) SKR_NOEXCEPT = 0;
};

}
#endif

RUNTIME_EXTERN_C RUNTIME_API
skr_system_handler_id skr_system_get_default_handler();

RUNTIME_EXTERN_C RUNTIME_API
void skr_system_pump_messages(skr_system_handler_id handler, float delta);

RUNTIME_EXTERN_C RUNTIME_API
void skr_system_process_messages(skr_system_handler_id handler, float delta);

RUNTIME_EXTERN_C RUNTIME_API
void skr_system_update_device_states(skr_system_handler_id handler);

RUNTIME_EXTERN_C RUNTIME_API
void* skr_system_load_handler_proc(skr_system_handler_id handler, const char* name);

RUNTIME_EXTERN_C RUNTIME_API
void skr_system_add_message_handler(skr_system_handler_id handler, skr_system_message_handler_id message_handler);

RUNTIME_EXTERN_C RUNTIME_API
void skr_system_remove_message_handler(skr_system_handler_id handler, skr_system_message_handler_id message_handler);

RUNTIME_EXTERN_C RUNTIME_API
int64_t skr_system_add_window_resize_handler(skr_system_handler_id handler, SWindowResizeHandlerProc proc, void* usr_data);

RUNTIME_EXTERN_C RUNTIME_API
void skr_system_remove_window_resize_handler(skr_system_handler_id handler, int64_t rid);

RUNTIME_EXTERN_C RUNTIME_API
int64_t skr_system_add_window_close_handler(skr_system_handler_id handler, SWindowCloseHandlerProc proc, void* usr_data);

RUNTIME_EXTERN_C RUNTIME_API
void skr_system_remove_window_close_handler(skr_system_handler_id handler, int64_t rid);

RUNTIME_EXTERN_C RUNTIME_API
int64_t skr_system_add_window_move_handler(skr_system_handler_id handler, SWindowMoveHandlerProc proc, void* usr_data);

RUNTIME_EXTERN_C RUNTIME_API
void skr_system_remove_window_move_handler(skr_system_handler_id handler, int64_t rid);

RUNTIME_EXTERN_C RUNTIME_API
int64_t skr_system_add_mouse_wheel_handler(skr_system_handler_id handler, SMouseWheelHandlerProc proc, void* usr_data);

RUNTIME_EXTERN_C RUNTIME_API
void skr_system_remove_mouse_wheel_handler(skr_system_handler_id handler, int64_t rid);

RUNTIME_EXTERN_C RUNTIME_API
int64_t skr_system_add_mouse_button_down_handler(skr_system_handler_id handler, SMouseButtonDownHandlerProc proc, void* usr_data);

RUNTIME_EXTERN_C RUNTIME_API
void skr_system_remove_mouse_button_down_handler(skr_system_handler_id handler, int64_t rid);

RUNTIME_EXTERN_C RUNTIME_API
int64_t skr_system_add_mouse_button_up_handler(skr_system_handler_id handler, SMouseButtonUpHandlerProc proc, void* usr_data);

RUNTIME_EXTERN_C RUNTIME_API
void skr_system_remove_mouse_button_up_handler(skr_system_handler_id handler, int64_t rid);

RUNTIME_EXTERN_C RUNTIME_API
int64_t skr_system_add_key_down_handler(skr_system_handler_id handler, SKeyDownHandlerProc proc, void* usr_data);

RUNTIME_EXTERN_C RUNTIME_API
void skr_system_remove_key_down_handler(skr_system_handler_id handler, int64_t rid);

RUNTIME_EXTERN_C RUNTIME_API
int64_t skr_system_add_key_up_handler(skr_system_handler_id handler, SKeyUpHandlerProc proc, void* usr_data);

RUNTIME_EXTERN_C RUNTIME_API
void skr_system_remove_key_up_handler(skr_system_handler_id handler, int64_t rid);

RUNTIME_EXTERN_C RUNTIME_API
int64_t skr_system_add_text_input_handler(skr_system_handler_id handler, STextInputHandlerProc proc, void* usr_data);

RUNTIME_EXTERN_C RUNTIME_API
void skr_system_remove_text_input_handler(skr_system_handler_id handler, int64_t rid);