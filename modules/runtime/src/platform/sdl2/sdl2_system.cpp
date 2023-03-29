#include "./../system.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

namespace skr {
struct SDL2Message
{
    SDL_Event event;
};

struct SystemHandler_SDL2 : public SystemHandlerBase
{
    void pump_messages(float delta) SKR_NOEXCEPT final;
    void process_messages(float delta) SKR_NOEXCEPT final;
    void update_device_states() SKR_NOEXCEPT final;
    void* load_handler_proc(const char* name) SKR_NOEXCEPT final;

    static SystemHandler_SDL2* Get()
    {
        static SystemHandler_SDL2 handler;
        return &handler;
    }

    EMouseKey translate_button(uint32_t SDLButton)
    {
        EMouseKey key = MOUSE_KEY_None;
        if (SDLButton == SDL_BUTTON_LEFT) key = MOUSE_KEY_LB;
        else if (SDLButton == SDL_BUTTON_RIGHT) key = MOUSE_KEY_RB;
        else if (SDLButton == SDL_BUTTON_MIDDLE) key = MOUSE_KEY_MB;
        else if (SDLButton == SDL_BUTTON_X1) key = MOUSE_KEY_X1B;
        else if (SDLButton == SDL_BUTTON_X2) key = MOUSE_KEY_X2B;
        return key;
    }

    skr::vector<SDL2Message> messages;
};

void SystemHandler_SDL2::pump_messages(float delta) SKR_NOEXCEPT
{
    SDL2Message message;
    while (SDL_PollEvent(&message.event))
    {
        messages.emplace_back(message);
    }
}

void SystemHandler_SDL2::process_messages(float delta) SKR_NOEXCEPT
{
    skr::vector<SDL2Message> to_process = {};
    to_process.swap(messages);
    for (auto message : to_process)
    {
    const auto& event = message.event;
    switch (event.type)
    {
    case SDL_WINDOWEVENT:
    {
        uint8_t window_event = event.window.event;
        auto sdl_window = SDL_GetWindowFromID(event.window.windowID);
        if (window_event == SDL_WINDOWEVENT_CLOSE)
            message_handler_proxy.on_window_closed((SWindowHandle)sdl_window);
        else if (window_event == SDL_WINDOWEVENT_MOVED)
            message_handler_proxy.on_window_moved((SWindowHandle)sdl_window);
        else if (window_event == SDL_WINDOWEVENT_RESIZED || window_event == SDL_WINDOWEVENT_SIZE_CHANGED)
            message_handler_proxy.on_window_resized((SWindowHandle)sdl_window, event.window.data1, event.window.data2);
    }
    break;
    
    case SDL_MOUSEWHEEL:
    {
        message_handler_proxy.on_mouse_wheel(event.wheel.x, event.wheel.y);
    }
    break;

    case SDL_MOUSEBUTTONUP:
    {
        message_handler_proxy.on_mouse_button_up(translate_button(event.button.button), event.button.x, event.button.y);
    }
    break;

    case SDL_MOUSEBUTTONDOWN:
    {
        message_handler_proxy.on_mouse_button_down(translate_button(event.button.button), event.button.x, event.button.y);
    }
    break;

    default: break;
    }
    }
}

void SystemHandler_SDL2::update_device_states() SKR_NOEXCEPT
{

}

void* SystemHandler_SDL2::load_handler_proc(const char* name) SKR_NOEXCEPT
{
    return nullptr;
}

}