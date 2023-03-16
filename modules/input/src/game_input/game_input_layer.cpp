#include "SkrInput/input.h"
#include "platform/memory.h"
#include "platform/atomic.h"
#include "platform/debug.h"
#include "utils/log.h"
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
        return this->Initialized;
    }
    
    bool Finalize() SKR_NOEXCEPT
    {
        return true;
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