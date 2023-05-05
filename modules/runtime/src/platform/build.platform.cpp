#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h> 
#endif
#include "platform/configure.h"
#include "debug.cpp"
#include "vfs.cpp"
#include "standard/stdio_vfs.cpp"
#include "guid.cpp"
#ifdef SKR_OS_UNIX
    #include "unix/unix_vfs.cpp"
    #include "unix/process.cpp"
#elif defined(SKR_OS_WINDOWS)
    #include "windows/windows_vfs.cpp"
    #include "windows/process.cpp"
#endif

#include "platform/system.h"
namespace skr
{
ISystemMessageHandler::~ISystemMessageHandler() SKR_NOEXCEPT {}
ISystemHandler::~ISystemHandler() SKR_NOEXCEPT {}
}

#ifdef RUNTIME_SHARED
extern "C" {
RUNTIME_API bool mi_allocator_init(const char** message)
{
    if (message != NULL) *message = NULL;
    return true;
}

RUNTIME_API void mi_allocator_done(void)
{
    // nothing to do
}

void skr_system_pump_messages(skr_system_handler_id handler, float delta)
{
    handler->pump_messages(delta);
}

void skr_system_process_messages(skr_system_handler_id handler, float delta)
{
    handler->process_messages(delta);
}

void skr_system_update_device_states(skr_system_handler_id handler)
{
    handler->update_device_states();
}

void* skr_system_load_handler_proc(skr_system_handler_id handler, const char* name)
{
    return handler->load_handler_proc(name);
}

void skr_system_add_message_handler(skr_system_handler_id handler, skr_system_message_handler_id message_handler)
{
    handler->add_message_handler(message_handler);
}

void skr_system_remove_message_handler(skr_system_handler_id handler, skr_system_message_handler_id message_handler)
{
    handler->remove_message_handler(message_handler);
}

int64_t skr_system_add_window_resize_handler(skr_system_handler_id handler, SWindowResizeHandlerProc proc, void* usr_data)
{
    return handler->add_window_resize_handler(proc, usr_data);
}

void skr_system_remove_window_resize_handler(skr_system_handler_id handler, int64_t rid)
{
    handler->remove_window_resize_handler(rid);
}

int64_t skr_system_add_window_close_handler(skr_system_handler_id handler, SWindowCloseHandlerProc proc, void* usr_data)
{
    return handler->add_window_close_handler(proc, usr_data);
}

void skr_system_remove_window_close_handler(skr_system_handler_id handler, int64_t rid)
{
    handler->remove_window_close_handler(rid);
}

int64_t skr_system_add_window_move_handler(skr_system_handler_id handler, SWindowMoveHandlerProc proc, void* usr_data)
{
    return handler->add_window_move_handler(proc, usr_data);
}

void skr_system_remove_window_move_handler(skr_system_handler_id handler, int64_t rid)
{
    handler->remove_window_move_handler(rid);
}

int64_t skr_system_add_mouse_wheel_handler(skr_system_handler_id handler, SMouseWheelHandlerProc proc, void* usr_data)
{
    return handler->add_mouse_wheel_handler(proc, usr_data);
}

void skr_system_remove_mouse_wheel_handler(skr_system_handler_id handler, int64_t rid)
{
    handler->remove_mouse_wheel_handler(rid);
}

int64_t skr_system_add_mouse_button_down_handler(skr_system_handler_id handler, SMouseButtonDownHandlerProc proc, void* usr_data)
{
    return handler->add_mouse_button_down_handler(proc, usr_data);
}

void skr_system_remove_mouse_button_down_handler(skr_system_handler_id handler, int64_t rid)
{
    handler->remove_mouse_button_down_handler(rid);
}

int64_t skr_system_add_mouse_button_up_handler(skr_system_handler_id handler, SMouseButtonUpHandlerProc proc, void* usr_data)
{
    return handler->add_mouse_button_up_handler(proc, usr_data);
}

void skr_system_remove_mouse_button_up_handler(skr_system_handler_id handler, int64_t rid)
{
    handler->remove_mouse_button_up_handler(rid);
}

int64_t skr_system_add_key_down_handler(skr_system_handler_id handler, SKeyDownHandlerProc proc, void* usr_data)
{
    return handler->add_key_down_handler(proc, usr_data);
}

void skr_system_remove_key_down_handler(skr_system_handler_id handler, int64_t rid)
{
    handler->remove_key_down_handler(rid);
}

int64_t skr_system_add_key_up_handler(skr_system_handler_id handler, SKeyUpHandlerProc proc, void* usr_data)
{
    return handler->add_key_up_handler(proc, usr_data);
}

void skr_system_remove_key_up_handler(skr_system_handler_id handler, int64_t rid)
{
    handler->remove_key_up_handler(rid);
}

int64_t skr_system_add_text_input_handler(skr_system_handler_id handler, STextInputHandlerProc proc, void* usr_data)
{
    return handler->add_text_input_handler(proc, usr_data);
}

void skr_system_remove_text_input_handler(skr_system_handler_id handler, int64_t rid)
{
    handler->remove_text_input_handler(rid);
}

}
#endif

#include "sdl2/sdl2_system.cpp"
RUNTIME_EXTERN_C RUNTIME_API
skr_system_handler_id skr_system_get_default_handler()
{
    return skr::SystemHandler_SDL2::Get();
}
