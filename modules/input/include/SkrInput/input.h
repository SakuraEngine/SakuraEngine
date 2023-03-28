#pragma once
#include "SkrInput/module.configure.h"
#include "containers/lite.hpp"
#include "platform/input.h"

SKR_DECLARE_TYPE_ID_FWD(skr::input, InputDevice, skr_input_device)
SKR_DECLARE_TYPE_ID_FWD(skr::input, InputReading, skr_input_reading)

namespace skr {
namespace input {

typedef enum EInputKind
{
    InputKindUnknown          = 0x00000000,
    InputKindRawDeviceReport  = 0x00000001,
    InputKindControllerAxis   = 0x00000002,
    InputKindControllerButton = 0x00000004,
    InputKindControllerSwitch = 0x00000008,
    InputKindController       = 0x0000000E,
    InputKindKeyboard         = 0x00000010,
    InputKindMouse            = 0x00000020,
    InputKindTouch            = 0x00000100,
    InputKindMotion           = 0x00001000,
    InputKindArcadeStick      = 0x00010000,
    InputKindFlightStick      = 0x00020000,
    InputKindGamepad          = 0x00040000,
    InputKindRacingWheel      = 0x00080000,
    InputKindUiNavigation     = 0x01000000
} EInputKind;

typedef enum InputMouseButtonFlags
{
    InputMouseNone           = 0x00000000,
    InputMouseLeftButton     = 0x00000001,
    InputMouseRightButton    = 0x00000002,
    InputMouseMiddleButton   = 0x00000004,
    InputMouseButton4        = 0x00000008,
    InputMouseButton5        = 0x00000010,
    // InputMouseWheelTiltLeft  = 0x00000020,
    // InputMouseWheelTiltRight = 0x00000040
} InputMouseButtonFlags;
typedef uint32_t InputMouseButtons;

struct InputKeyState
{
    // uint32_t scan_code;  
    // uint32_t code_point;  
    SInputKeyCode virtual_key;  
    bool is_dead_key;  
};

struct InputMouseState
{
    InputMouseButtons buttons;
    int64_t positionX;
    int64_t positionY;
    int64_t wheelX;
    int64_t wheelY;
};

typedef enum EInputResult
{
    INPUT_RESULT_OK,
    INPUT_RESULT_FAIL,
    INPUT_RESULT_NOT_FOUND
} EInputResult;

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
    virtual void Tick() SKR_NOEXCEPT {}
    virtual uint64_t GetReadingHistoryLifetimeUSec() const SKR_NOEXCEPT = 0;

    virtual uint64_t GetCurrentTimestampUSec() SKR_NOEXCEPT = 0;
    virtual EInputResult GetCurrentReading(EInputKind kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT = 0;
    virtual EInputResult GetNextReading(InputReading* reference, EInputKind kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT = 0;
    virtual EInputResult GetPreviousReading(InputReading* reference, EInputKind kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT = 0;

    virtual void GetDevice(InputReading* reading, InputDevice** out_device) SKR_NOEXCEPT = 0;
    virtual uint32_t GetKeyState(InputReading* reading, uint32_t stateArrayCount, InputKeyState* stateArray) SKR_NOEXCEPT = 0;
    virtual bool GetMouseState(InputReading* reading, InputMouseState* state) SKR_NOEXCEPT = 0;
    virtual uint64_t GetTimestampUSec(InputReading* reading) SKR_NOEXCEPT = 0;

    virtual void Release(InputReading* reading) SKR_NOEXCEPT = 0;
    virtual void Release(InputDevice* device) SKR_NOEXCEPT = 0;
};

struct SKR_INPUT_API Input
{
    Input() SKR_NOEXCEPT;
    virtual ~Input() SKR_NOEXCEPT;

    static void Initialize() SKR_NOEXCEPT;
    static void Finalize() SKR_NOEXCEPT;
    static Input* GetInstance() SKR_NOEXCEPT;

    EInputResult GetCurrentReading(EInputKind kind, InputDevice* device, InputLayer** out_layer, InputReading** out_reading);
    EInputResult GetNextReading(InputReading* reference, EInputKind kind, InputDevice* device, InputLayer** out_layer, InputReading** out_reading);
    EInputResult GetPreviousReading(InputReading* reference, EInputKind kind, InputDevice* device, InputLayer** out_layer, InputReading** out_reading);
    
    virtual void Tick() SKR_NOEXCEPT
    {
        for (auto layer : GetLayers())
        {
            if (layer->IsEnabled())
            {
                layer->Tick();
            }
        }
    }
    lite::LiteSpan<InputLayer*> GetLayers() SKR_NOEXCEPT;

protected:
    static Input* instance_;
};

} }