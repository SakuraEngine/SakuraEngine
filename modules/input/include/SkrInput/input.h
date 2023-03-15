#pragma once
#include "SkrInput/module.configure.h"
#include "platform/configure.h"

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

struct SKR_INPUT_API Input
{
    Input() SKR_NOEXCEPT;
    virtual ~Input() SKR_NOEXCEPT;

    static void Initialize() SKR_NOEXCEPT;
    static void Finalize() SKR_NOEXCEPT;
    static Input* GetInstance() SKR_NOEXCEPT;

protected:
    static Input* instance_;
};

SKR_INPUT_API int32_t GetCurrentReading(EInputKind kind, InputDevice* device, InputReading** out_reading);
SKR_INPUT_API int32_t GetNextReading(InputReading* reference, EInputKind kind, InputDevice* device, InputReading** out_reading);
SKR_INPUT_API int32_t GetPreviousReading(InputReading* reference, EInputKind kind, InputDevice* device, InputReading** out_reading);

} }