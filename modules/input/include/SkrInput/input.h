#pragma once
#include "SkrInput/module.configure.h"
#include "containers/lite.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::input, InputDevice, skr_input_device)
SKR_DECLARE_TYPE_ID_FWD(skr::input, InputReading, skr_input_reading)

namespace skr {
namespace input {

typedef enum EInputKind
{
    InputKindUnknown         = 0x00000000,
    InputKindRawDeviceReport = 0x00000001,
    InputKindController      = 0x00000002,
    InputKindKeyboard        = 0x00000004,
    InputKindMouse           = 0x00000008,
    InputKindTouch           = 0x00000100,
    InputKindMotion          = 0x00001000,
    InputKindArcadeStick     = 0x00010000,
    InputKindFlightStick     = 0x00020000,
    InputKindGamepad         = 0x00040000,
    InputKindRacingWheel     = 0x00080000,
    InputKindUiNavigation    = 0x01000000
} InputKind;

using LayerId = skr_guid_t;
static const LayerId kGameInputLayerId = {0xa0bb28b1, 0xacdb, 0x41fb, {0x87, 0xaa, 0x9d, 0x09, 0xfb, 0x92, 0x31, 0x8f}};
static const LayerId kCommonInputLayerId = {0x1b1487f5, 0x7850, 0x4b85, {0x9f, 0xc3, 0x0a, 0x9f, 0x81, 0x28, 0xcc, 0x5a}};
struct SKR_INPUT_API InputLayer
{
    virtual ~InputLayer() SKR_NOEXCEPT;

    virtual void GetLayerId(LayerId* out_id) const SKR_NOEXCEPT = 0;
    virtual bool Initialize() SKR_NOEXCEPT = 0;
    virtual bool Finalize() SKR_NOEXCEPT = 0;
    virtual bool SetEnabled(bool enabled) SKR_NOEXCEPT = 0;
    virtual bool IsEnabled() const SKR_NOEXCEPT = 0;
};

struct SKR_INPUT_API Input
{
    Input() SKR_NOEXCEPT;
    virtual ~Input() SKR_NOEXCEPT;

    static void Initialize() SKR_NOEXCEPT;
    static void Finalize() SKR_NOEXCEPT;
    static Input* GetInstance() SKR_NOEXCEPT;

    lite::LiteSpan<InputLayer*> GetLayers() SKR_NOEXCEPT;

protected:
    static Input* instance_;
};

SKR_INPUT_API int32_t GetCurrentReading(EInputKind kind, InputDevice* device, InputReading** out_reading);
SKR_INPUT_API int32_t GetNextReading(InputReading* reference, EInputKind kind, InputDevice* device, InputReading** out_reading);
SKR_INPUT_API int32_t GetPreviousReading(InputReading* reference, EInputKind kind, InputDevice* device, InputReading** out_reading);

} }