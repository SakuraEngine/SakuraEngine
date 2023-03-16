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

struct Input_GameInput : public InputLayer
{
    bool DoCreate() SKR_NOEXCEPT
    {
        if (pGameInputCreate)
        {
            const uint32_t bUsingRedist = (pGameInputCreate != &GameInputCreate) ? 1 : 0;
            auto CreateResult = pGameInputCreate(&game_input);
            if (!SUCCEEDED(CreateResult))
            {
                if (pGameInputCreate == &GameInputCreate) // Proc from redist
                {
                    SKR_LOG_ERROR("GameInput: XCurl is loaded, but GameInputCreated failed to create! HRESULT: 0x%08X", HRESULT_FROM_WIN32(GetLastError()));
                    
                    if (GameInputRedist.isLoaded()) GameInputRedist.unload();
                    if (GameInput.isLoaded()) GameInput.unload();
                }
                else
                {
                    SKR_LOG_ERROR("GameInput: GameInputRedist is loaded, but GameInputCreated failed to create! HRESULT: 0x%08X", HRESULT_FROM_WIN32(GetLastError()));

                    if (GDKThunks.isLoaded()) GDKThunks.unload();
                    if (XCurl.isLoaded()) XCurl.unload();
                }
            }
            else
            {
                const char* names[] = { "XCurl", "GameInputRedist" };
                SKR_LOG_INFO("GameInput: initialized with %s successfully!", names[bUsingRedist]);
                return true;
            }
        }
        return false;
    }

    bool CreateWithGameInputRedist() SKR_NOEXCEPT
    {
        auto GameInputRedistLoaded = GameInputRedist.load("gameinputredist.dll");
        auto GameInputLoaded = GameInput.load("gameinput.dll");
        if (GameInputRedistLoaded && GameInputLoaded)
        {
            SKR_LOG_INFO("GameInput: GameInputRedist Loaded!");
            pGameInputCreate = (ProcType*)GameInput.getRawAddress("GameInputCreate");
        }
        else
        {
            if (GameInputRedistLoaded && !GameInputLoaded)
            {
                SKR_LOG_ERROR("GameInput: GameInputRedist is loaded, but GameInput is not!");
                GameInputRedist.unload();
            }
            if (!GameInputRedistLoaded && GameInputLoaded)
            {
                SKR_LOG_ERROR("GameInput: GameInput is loaded, but GameInputRedist is not!");
                GameInput.unload();
            }
            return false;
        }
        return DoCreate();
    }

    bool CreateWithXCurl() SKR_NOEXCEPT
    {
        auto GDKThunksLoaded = GDKThunks.load("Microsoft.Xbox.Services.141.GDK.C.Thunks.dll");
        auto XCurlLoaded = XCurl.load("XCurl.dll");
        if (GDKThunksLoaded && XCurlLoaded)
        {
            SKR_LOG_INFO("GameInput: GDKThunks & XCurl Loaded!");
            pGameInputCreate = &GameInputCreate;
        }
        else
        {
            if (GDKThunksLoaded && !XCurlLoaded)
            {
                SKR_LOG_ERROR("GameInput: GDKThunks is loaded, but XCurl is not!");
                GDKThunks.unload();
            }
            if (!GDKThunksLoaded && XCurlLoaded)
            {
                SKR_LOG_ERROR("GameInput: XCurl is loaded, but GDKThunks is not!");
                XCurl.unload();
            }
            return false;
        }
        return DoCreate();
    }

    Input_GameInput() SKR_NOEXCEPT
        : InputLayer()
    {
        if (!CreateWithXCurl())
        {
            SKR_LOG_INFO("GameInput: Retry initialization using GameInputRedist");
            if (!CreateWithGameInputRedist())
            {
                SKR_LOG_ERROR("GameInput: Failed to create with both XCurl and GameInputRedist");
                Initialized = false;
            }
        }
    }

    ~Input_GameInput() SKR_NOEXCEPT
    {
        if (game_input)
        {
            game_input->Release();
        }
        if (GameInputRedist.isLoaded()) GameInputRedist.unload();
        if (GameInput.isLoaded()) GameInput.unload();
        if (GDKThunks.isLoaded()) GDKThunks.unload();
        if (XCurl.isLoaded()) XCurl.unload();
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

    EInputResult GetCurrentReading(EInputKind kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT final
    {
        if (game_input)
        {
            IGameInputReading* reading = nullptr;
            auto hr = game_input->GetCurrentReading((GameInputKind)kind, (IGameInputDevice*)device, &reading);
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
                stateArray[i].scan_code = states[i].scanCode;
                stateArray[i].code_point = states[i].codePoint;
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

    using ProcType = decltype(GameInputCreate);
    SAtomic32 enabled = 1;
    ProcType* pGameInputCreate = nullptr;
    skr::SharedLibrary GDKThunks;
    skr::SharedLibrary XCurl;
    skr::SharedLibrary GameInputRedist;
    skr::SharedLibrary GameInput;
    IGameInput* game_input = nullptr;
    bool Initialized = true;
};

InputLayer* Input_GameInput_Create() SKR_NOEXCEPT
{
    return SkrNew<Input_GameInput>();
}

} }