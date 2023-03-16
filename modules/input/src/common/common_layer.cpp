#include "SkrInput/input.h"
#include "platform/memory.h"
#include "platform/atomic.h"
#include "platform/debug.h"
#include "platform/shared_library.hpp"

namespace skr {
namespace input {

// GameInput implementation
struct Input_Common : public InputLayer
{
    Input_Common() SKR_NOEXCEPT
        : InputLayer()
    {

    }

    ~Input_Common() SKR_NOEXCEPT
    {

    }

    void GetLayerId(LayerId* out_id) const SKR_NOEXCEPT final
    {
        if (out_id) *out_id = kCommonInputLayerId;
    }

    bool Initialize() SKR_NOEXCEPT final
    {
        return true;
    }
    
    bool Finalize() SKR_NOEXCEPT final
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

    uint64_t GetCurrentTimestampUSec() SKR_NOEXCEPT final
    {
        SKR_UNIMPLEMENTED_FUNCTION();
        return 0;
    }

    EInputResult GetCurrentReading(EInputKind kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT final
    {
        SKR_UNIMPLEMENTED_FUNCTION();
        return INPUT_RESULT_NOT_FOUND;
    }

    EInputResult GetNextReading(InputReading* reference, EInputKind kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT final
    {
        SKR_UNIMPLEMENTED_FUNCTION();
        return INPUT_RESULT_NOT_FOUND;
    }

    EInputResult GetPreviousReading(InputReading* reference, EInputKind kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT final
    {
        SKR_UNIMPLEMENTED_FUNCTION();
        return INPUT_RESULT_NOT_FOUND;
    }

    void GetDevice(InputReading* in_reading, InputDevice** out_device) SKR_NOEXCEPT final
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }

    uint32_t GetKeyState(InputReading* in_reading, uint32_t stateArrayCount, InputKeyState* stateArray) SKR_NOEXCEPT final
    {
        SKR_UNIMPLEMENTED_FUNCTION();
        return 0;
    }

    uint32_t GetMouseState(InputReading* in_reading, InputMouseState* state) SKR_NOEXCEPT final
    {
        SKR_UNIMPLEMENTED_FUNCTION();
        return 0;
    }

    virtual uint64_t GetTimestampUSec(InputReading* in_reading) SKR_NOEXCEPT final
    {
        SKR_UNIMPLEMENTED_FUNCTION();
        return 0;
    }

    void Release(InputReading* in_reading) SKR_NOEXCEPT final
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }

    void Release(InputDevice* in_device) SKR_NOEXCEPT final
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }

    SAtomic32 enabled = true;
};

InputLayer* Input_Common_Create() SKR_NOEXCEPT
{
    return SkrNew<Input_Common>();
}

} }