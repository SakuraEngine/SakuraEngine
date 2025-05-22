#include "../common/common_device_base.hpp"
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_events.h>
#include <algorithm>
#include "SkrContainers/span.hpp"

namespace skr
{
namespace input
{

struct MouseState {
    uint32_t ButtonFlags;
    int      x      = 0;
    int      y      = 0;
    int64_t  wheelX = 0;
    int64_t  wheelY = 0;
};

struct InputReading_SDL3Mouse : public CommonInputReading {
    InputReading_SDL3Mouse(CommonInputReadingProxy* pPool, struct CommonInputDevice* pDevice, const MouseState& State, uint64_t Timestamp) SKR_NOEXCEPT
        : CommonInputReading(pPool, pDevice),
          State(State),
          Timestamp(Timestamp)
    {
    }

    uint64_t GetTimestamp() const SKR_NOEXCEPT final
    {
        return Timestamp;
    }

    EInputKind GetInputKind() const SKR_NOEXCEPT
    {
        return EInputKind::InputKindMouse;
    }

    virtual uint32_t GetKeyState(uint32_t stateArrayCount, InputKeyState* stateArray) SKR_NOEXCEPT
    {
        return 0;
    }

    bool GetMouseState(InputMouseState* state) SKR_NOEXCEPT final
    {
        state->buttons = 0;
        state->buttons |= (State.ButtonFlags & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) ? InputMouseLeftButton : 0;
        state->buttons |= (State.ButtonFlags & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) ? InputMouseRightButton : 0;
        state->buttons |= (State.ButtonFlags & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)) ? InputMouseMiddleButton : 0;
        state->buttons |= (State.ButtonFlags & SDL_BUTTON_MASK(SDL_BUTTON_X1)) ? InputMouseButton4 : 0;
        state->buttons |= (State.ButtonFlags & SDL_BUTTON_MASK(SDL_BUTTON_X2)) ? InputMouseButton5 : 0;

        state->positionX = State.x;
        state->positionY = State.y;
        state->wheelX    = State.wheelX;
        state->wheelY    = State.wheelY;

        return true;
    }

    bool Equal(const MouseState& state)
    {
        return false; // GameInput behaves like this
    }

    MouseState State;
    uint64_t   Timestamp;
};

struct InputDevice_SDL3Mouse : public CommonInputDeviceBase<InputReading_SDL3Mouse> {
    InputDevice_SDL3Mouse(CommonInputLayer* Layer) SKR_NOEXCEPT
        : CommonInputDeviceBase<InputReading_SDL3Mouse>(Layer)
    {
    }

    void Tick() SKR_NOEXCEPT final
    {
        MouseState mouseState;
        updateScan(mouseState);
        const auto LastReading = ReadingQueue.get();
        if (!LastReading || !LastReading->Equal(mouseState))
        {
            if (auto old = ReadingQueue.add(
                ReadingPool.acquire(&ReadingPool, this, mouseState, layer->GetCurrentTimestampUSec())))
            {
                old->release();
            }
        }
    }

    const EInputKind       kinds[1] = { EInputKind::InputKindMouse };
    span<const EInputKind> ReportKinds() const SKR_NOEXCEPT final
    {
        return { kinds, 1 };
    }

    bool SupportKind(EInputKind kind) const SKR_NOEXCEPT final
    {
        return kind == EInputKind::InputKindMouse;
    }

    void updateScan(MouseState& outState)
    {
        float mouse_x, mouse_y;
        outState.ButtonFlags = SDL_GetMouseState(&mouse_x, &mouse_y);
        outState.x = (int)mouse_x;
        outState.y = (int)mouse_y;
        SDL_PumpEvents();
        SDL_Event events[8];
        if (auto count = SDL_PeepEvents(events, 8, SDL_GETEVENT, SDL_EVENT_MOUSE_WHEEL, SDL_EVENT_MOUSE_WHEEL))
        {
            for (int i = 0; i < count; i++)
            {
                SDL_MouseWheelEvent const& e = events[i].wheel;
                skr_atomic_fetch_add_relaxed(&wheelXAccum, e.x * 120);
                skr_atomic_fetch_add_relaxed(&wheelYAccum, e.y * 120);
            }
        }
        outState.wheelX = skr_atomic_load_relaxed(&wheelXAccum);
        outState.wheelY = skr_atomic_load_relaxed(&wheelYAccum);
    }

    SAtomic64 wheelXAccum = 0;
    SAtomic64 wheelYAccum = 0;
};

CommonInputDevice* CreateInputDevice_SDL3Mouse(CommonInputLayer* pLayer) SKR_NOEXCEPT
{
    InputDevice_SDL3Mouse* pDevice = SkrNew<InputDevice_SDL3Mouse>(pLayer);
    return pDevice;
}

} // namespace input
} // namespace skr