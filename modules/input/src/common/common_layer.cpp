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

    SAtomic32 enabled = true;
};

InputLayer* Input_Common_Create() SKR_NOEXCEPT
{
    return SkrNew<Input_Common>();
}

} }