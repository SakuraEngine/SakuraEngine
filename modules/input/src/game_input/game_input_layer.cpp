#include "SkrInput/input.h"
#include "platform/memory.h"
#include "platform/atomic.h"
#include "platform/debug.h"
#include "utils/log.h"
#include "utils/defer.hpp"
#include "GameInput/GameInput.h"
#include "platform/shared_library.hpp"

namespace skr {
namespace input {
// GameInput implementation

void CALLBACK OnDeviceEnumerated(
    _In_ GameInputCallbackToken callbackToken,
    _In_ void* context,
    _In_ IGameInputDevice* device,
    _In_ uint64_t timestamp,
    _In_ GameInputDeviceStatus currentStatus,
    _In_ GameInputDeviceStatus previousStatus)
{
    auto displayName = device->GetDeviceInfo()->displayName;
    const char* displayNameStr = displayName ? displayName->data : "Unknown";
    SKR_LOG_INFO("GameInput: Device %s Enumerated!", displayNameStr);
}

struct Input_GameInput : public InputLayer
{
    skr::SharedLibrary GameInputLibrary;
    bool DoCreate() SKR_NOEXCEPT
    {
        if (auto GameInputLoaded = GameInputLibrary.load("GameInput.dll"); GameInputLoaded)
        {
            using FuncType = decltype(GameInputCreate);
            auto pCreateFunc = (FuncType*)GameInputLibrary.getRawAddress("GameInputCreate");
            if (pCreateFunc)
            {
                auto CreateResult = pCreateFunc(&game_input);
                if (SUCCEEDED(CreateResult)) 
                {
                    return true;
                }
            }
        }
        auto CreateResult = GameInputCreate(&game_input);
        return SUCCEEDED(CreateResult);
    }

    Input_GameInput() SKR_NOEXCEPT
        : InputLayer()
    {
        if (DoCreate())
        {
            // Find connected devices
            GameInputCallbackToken token;
            if (SUCCEEDED(game_input->RegisterDeviceCallback(
                nullptr,
                GameInputKindKeyboard,
                GameInputDeviceAnyStatus,
                GameInputBlockingEnumeration,
                nullptr,
                OnDeviceEnumerated,
                &token)))
            {
                game_input->UnregisterCallback(token, 5000);
            }
        }
        else
        {
            SKR_LOG_ERROR("GameInput: Failed to create with both XCurl and GameInputRedist");
            Initialized = false;
        }
    }

    ~Input_GameInput() SKR_NOEXCEPT
    {
        if (game_input)
        {
            game_input->Release();
        }
        if (GameInputLibrary.isLoaded())
        {
            GameInputLibrary.unload();
        } 
    }

    void GetLayerId(LayerId* out_id) const SKR_NOEXCEPT final
    {
        if (out_id) *out_id = kGameInputLayerId;
    }

    bool Initialize() SKR_NOEXCEPT
    {
        game_input->SetFocusPolicy(GameInputDisableBackgroundInput);
        return this->Initialized;
    }
    
    bool Finalize() SKR_NOEXCEPT
    {
        return true;
    }

    void Tick() SKR_NOEXCEPT final
    {

    }

    bool SetEnabled(bool _enabled) SKR_NOEXCEPT final
    {
        skr_atomic32_store_release(&enabled, _enabled ? 1 : 0);
        return true;
    }
    
    bool IsEnabled() const SKR_NOEXCEPT final
    {
        auto enabled_val = skr_atomic32_load_acquire(&enabled);
        return enabled_val;
    }

    uint64_t GetCurrentTimestampUSec() SKR_NOEXCEPT final
    {
        return game_input ? game_input->GetCurrentTimestamp() : 0;
    }

    EInputResult GetCurrentReading(EInputKind in_kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT final
    {
        if (game_input)
        {
            IGameInputReading* reading = nullptr;
            GameInputKind kind = (GameInputKind)in_kind;
            auto hr = game_input->GetCurrentReading(kind, (IGameInputDevice*)device, &reading);
            if (SUCCEEDED(hr))
            {
                *out_reading = (InputReading*)reading;
                return INPUT_RESULT_OK;
            }
        }
        return INPUT_RESULT_NOT_FOUND;
    }

    EInputResult GetNextReading(InputReading* reference, EInputKind kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT final
    {
        if (game_input)
        {
            IGameInputReading* reading = nullptr;
            auto hr = game_input->GetNextReading((IGameInputReading*)reference, (GameInputKind)kind, (IGameInputDevice*)device, &reading);
            if (SUCCEEDED(hr))
            {
                *out_reading = (InputReading*)reading;
                return INPUT_RESULT_OK;
            }
        }
        return INPUT_RESULT_NOT_FOUND;
    }

    EInputResult GetPreviousReading(InputReading* reference, EInputKind kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT final
    {
        if (game_input)
        {
            IGameInputReading* reading = nullptr;
            auto hr = game_input->GetPreviousReading((IGameInputReading*)reference, (GameInputKind)kind, (IGameInputDevice*)device, &reading);
            if (SUCCEEDED(hr))
            {
                *out_reading = (InputReading*)reading;
                return INPUT_RESULT_OK;
            }
        }
        return INPUT_RESULT_NOT_FOUND;
    }

    void GetDevice(InputReading* in_reading, InputDevice** out_device) SKR_NOEXCEPT final
    {
        if (auto reading = (IGameInputReading*)in_reading)
        {
            IGameInputDevice* device = nullptr;
            reading->GetDevice(&device);
            *out_device = (InputDevice*)device;
        }
    }

    uint32_t GetKeyState(InputReading* in_reading, uint32_t stateArrayCount, InputKeyState* stateArray) SKR_NOEXCEPT final
    {
        if (auto reading = (IGameInputReading*)in_reading)
        {
            GameInputKeyState states[16];
            uint32_t readCount = reading->GetKeyState(stateArrayCount, states);
            for (uint32_t i = 0; i < readCount; i++)
            {
                // stateArray[i].scan_code = states[i].scanCode;
                // stateArray[i].code_point = states[i].codePoint;
                stateArray[i].virtual_key = states[i].virtualKey;
                stateArray[i].is_dead_key = states[i].isDeadKey;
            }
            return readCount;
        }
        return 0;
    }

    uint32_t GetMouseState(InputReading* in_reading, InputMouseState* out_state) SKR_NOEXCEPT final
    {
        if (auto reading = (IGameInputReading*)in_reading)
        {
            GameInputMouseState state;
            reading->GetMouseState(&state);
            if (out_state)
            {
                out_state->buttons = (InputMouseButtons)state.buttons;
                out_state->positionX = state.positionX;
                out_state->positionY = state.positionY;
                out_state->wheelX = state.wheelX;
                out_state->wheelY = state.wheelY;
            }
            return 1;
        }
        return 0;
    }

    virtual uint64_t GetTimestampUSec(InputReading* in_reading) SKR_NOEXCEPT final
    {
        if (auto reading = (IGameInputReading*)in_reading)
        {
            return reading->GetTimestamp();
        }
        return 0;
    }

    void Release(InputReading* in_reading) SKR_NOEXCEPT final
    {
        if (auto reading = (IGameInputReading*)in_reading)
        {
            reading->Release();
        }
    }

    void Release(InputDevice* in_device) SKR_NOEXCEPT final
    {
        if (auto device = (IGameInputDevice*)in_device)
        {
            device->Release();
        }
    }

    uint64_t GetReadingHistoryLifetimeUSec() const SKR_NOEXCEPT final
    {
        return 500 * 1000;
    }

    using ProcType = decltype(GameInputCreate);
    SAtomic32 enabled = 1;
    IGameInput* game_input = nullptr;
    bool Initialized = true;
};

InputLayer* Input_GameInput_Create() SKR_NOEXCEPT
{
    return SkrNew<Input_GameInput>();
}

} }