#include "SkrRT/platform/configure.h"
#include "debug.cpp"
#include "vfs.cpp"
#include "guid.cpp"

#include "standard/stdio_vfs.cpp"
#include "standard/crash_handler.cpp"
#if defined(SKR_OS_UNIX)
    #if defined(SKR_OS_MACOSX)
        #define UNIX_CRASH_HANDLER_IMPLEMENTED
        #include "apple/crash_handler.cpp"
    #elif defined(SKR_OS_LINUX)
        #define UNIX_CRASH_HANDLER_IMPLEMENTED
        #include "linux/crash_handler.cpp"
    #endif
    #include "unix/unix_vfs.cpp"
    #include "unix/process.cpp"
    #include "unix/crash_handler.cpp"
#elif defined(SKR_OS_WINDOWS)
    #include "windows/windows_vfs.cpp"
    #include "windows/process.cpp"
    #include "windows/crash_handler.cpp"
#endif

#if defined(SKR_OS_WINDOWS)
    #include "windows/windows_dstorage.cpp"
    #include "windows/windows_dstorage_decompress.cpp"
#else
    #include "null/null_dstorage.cpp"
#endif

#include "SkrRT/platform/system.h"
namespace skr
{
ISystemMessageHandler::~ISystemMessageHandler() SKR_NOEXCEPT {}
ISystemHandler::~ISystemHandler() SKR_NOEXCEPT {}
}

#ifdef RUNTIME_SHARED
extern "C" {
SKR_RUNTIME_API bool mi_allocator_init(const char** message)
{
    if (message != NULL) *message = NULL;
    return true;
}

SKR_RUNTIME_API void mi_allocator_done(void)
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
SKR_EXTERN_C SKR_RUNTIME_API
skr_system_handler_id skr_system_get_default_handler()
{
    return skr::SystemHandler_SDL2::Get();
}
