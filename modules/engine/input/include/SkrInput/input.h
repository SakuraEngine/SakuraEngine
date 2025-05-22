#pragma once
#include "SkrBase/config.h"
#include "SkrContainers/span.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::input, InputDevice, skr_input_device)
SKR_DECLARE_TYPE_ID_FWD(skr::input, InputReading, skr_input_reading)

namespace skr
{
namespace input
{

typedef enum EKeyCode
{
    KEY_CODE_Invalid     = 0x00,
    KEY_CODE_Backspace   = 0x08,
    KEY_CODE_Tab         = 0x09,
    KEY_CODE_Clear       = 0x0C,
    KEY_CODE_Enter       = 0x0D,
    KEY_CODE_Shift       = 0x10,
    KEY_CODE_Ctrl        = 0x11,
    KEY_CODE_Alt         = 0x12,
    KEY_CODE_Pause       = 0x13,
    KEY_CODE_CapsLock    = 0x14,
    KEY_CODE_Esc         = 0x1B,
    KEY_CODE_SpaceBar    = 0x20,
    KEY_CODE_PageUp      = 0x21,
    KEY_CODE_PageDown    = 0x22,
    KEY_CODE_End         = 0x23,
    KEY_CODE_Home        = 0x24,
    KEY_CODE_Left        = 0x25,
    KEY_CODE_Up          = 0x26,
    KEY_CODE_Right       = 0x27,
    KEY_CODE_Down        = 0x28,
    KEY_CODE_Select      = 0x29,
    KEY_CODE_Print       = 0x2A,
    KEY_CODE_Execute     = 0x2B,
    KEY_CODE_Printscreen = 0x2C,
    KEY_CODE_Insert      = 0x2D,
    KEY_CODE_Del         = 0x2E,
    KEY_CODE_Help        = 0x2F,
    KEY_CODE_0           = 0x30,
    KEY_CODE_1           = 0x31,
    KEY_CODE_2           = 0x32,
    KEY_CODE_3           = 0x33,
    KEY_CODE_4           = 0x34,
    KEY_CODE_5           = 0x35,
    KEY_CODE_6           = 0x36,
    KEY_CODE_7           = 0x37,
    KEY_CODE_8           = 0x38,
    KEY_CODE_9           = 0x39,
    KEY_CODE_A           = 0x41,
    KEY_CODE_B           = 0x42,
    KEY_CODE_C           = 0x43,
    KEY_CODE_D           = 0x44,
    KEY_CODE_E           = 0x45,
    KEY_CODE_F           = 0x46,
    KEY_CODE_G           = 0x47,
    KEY_CODE_H           = 0x48,
    KEY_CODE_I           = 0x49,
    KEY_CODE_J           = 0x4A,
    KEY_CODE_K           = 0x4B,
    KEY_CODE_L           = 0x4C,
    KEY_CODE_M           = 0x4D,
    KEY_CODE_N           = 0x4E,
    KEY_CODE_O           = 0x4F,
    KEY_CODE_P           = 0x50,
    KEY_CODE_Q           = 0x51,
    KEY_CODE_R           = 0x52,
    KEY_CODE_S           = 0x53,
    KEY_CODE_T           = 0x54,
    KEY_CODE_U           = 0x55,
    KEY_CODE_V           = 0x56,
    KEY_CODE_W           = 0x57,
    KEY_CODE_X           = 0x58,
    KEY_CODE_Y           = 0x59,
    KEY_CODE_Z           = 0x5A,
    KEY_CODE_LSystem     = 0x5B,
    KEY_CODE_RSystem     = 0x5C,
    KEY_CODE_App         = 0x5D,
    KEY_CODE_Sleep       = 0x5F,
    KEY_CODE_Numpad0     = 0x60,
    KEY_CODE_Numpad1     = 0x61,
    KEY_CODE_Numpad2     = 0x62,
    KEY_CODE_Numpad3     = 0x63,
    KEY_CODE_Numpad4     = 0x64,
    KEY_CODE_Numpad5     = 0x65,
    KEY_CODE_Numpad6     = 0x66,
    KEY_CODE_Numpad7     = 0x67,
    KEY_CODE_Numpad8     = 0x68,
    KEY_CODE_Numpad9     = 0x69,
    KEY_CODE_Multiply    = 0x6A,
    KEY_CODE_Add         = 0x6B,
    KEY_CODE_Separator   = 0x6C,
    KEY_CODE_Subtract    = 0x6D,
    KEY_CODE_Decimal     = 0x6E,
    KEY_CODE_Divide      = 0x6F,
    KEY_CODE_F1          = 0x70,
    KEY_CODE_F2          = 0x71,
    KEY_CODE_F3          = 0x72,
    KEY_CODE_F4          = 0x73,
    KEY_CODE_F5          = 0x74,
    KEY_CODE_F6          = 0x75,
    KEY_CODE_F7          = 0x76,
    KEY_CODE_F8          = 0x77,
    KEY_CODE_F9          = 0x78,
    KEY_CODE_F10         = 0x79,
    KEY_CODE_F11         = 0x7A,
    KEY_CODE_F12         = 0x7B,
    KEY_CODE_F13         = 0x7C,
    KEY_CODE_F14         = 0x7D,
    KEY_CODE_F15         = 0x7E,
    KEY_CODE_F16         = 0x7F,
    KEY_CODE_F17         = 0x80,
    KEY_CODE_F18         = 0x81,
    KEY_CODE_F19         = 0x82,
    KEY_CODE_F20         = 0x83,
    KEY_CODE_F21         = 0x84,
    KEY_CODE_F22         = 0x85,
    KEY_CODE_F23         = 0x86,
    KEY_CODE_F24         = 0x87,
    KEY_CODE_Numlock     = 0x90,
    KEY_CODE_Scrolllock  = 0x91,
    KEY_CODE_LShift      = 0xA0,
    KEY_CODE_RShift      = 0xA1,
    KEY_CODE_LCtrl       = 0xA2,
    KEY_CODE_RCtrl       = 0xA3,
    KEY_CODE_LAlt        = 0xA4,
    KEY_CODE_RAlt        = 0xA5,
    KEY_CODE_Semicolon   = 0xBA, // ;: key on US standard keyboard
    KEY_CODE_Plus        = 0xBB, // =+ key
    KEY_CODE_Comma       = 0xBC, // ,< key
    KEY_CODE_Minus       = 0xBD, // -_ key
    KEY_CODE_Dot         = 0xBE, // .> key
    KEY_CODE_Slash       = 0xBF, // /? key on US standard keyboard
    KEY_CODE_Wave        = 0xC0, // ~` key on US standard keyboard
    KEY_CODE_LBracket    = 0xDB, // [{ key on US standard keyboard
    KEY_CODE_Backslash   = 0xDC, // \| key on US standard keyboard
    KEY_CODE_RBracket    = 0xDD, // ]} key on US standard keyboard
    KEY_CODE_Quote       = 0xDE, // '" key on US standard keyboard
} EKeyCode;
typedef uint8_t SInputKeyCode;

typedef enum EMouseKey
{
    MOUSE_KEY_None = 0x00,
    MOUSE_KEY_LB   = 0x01,
    MOUSE_KEY_RB   = 0x02,
    MOUSE_KEY_MB   = 0x04,
    MOUSE_KEY_X1B  = 0x08,
    MOUSE_KEY_X2B  = 0x10,
} EMouseKey;

typedef enum EMouseAxis
{
    MOUSE_AXIS_X        = 0x01,
    MOUSE_AXIS_Y        = 0x02,
    MOUSE_AXIS_XY       = MOUSE_AXIS_X | MOUSE_AXIS_Y,
    MOUSE_AXIS_WHEEL_X  = 0x04,
    MOUSE_AXIS_WHEEL_Y  = 0x08,
    MOUSE_AXIS_WHEEL_XY = MOUSE_AXIS_WHEEL_X | MOUSE_AXIS_WHEEL_Y
} EMouseAxis;

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
    InputMouseNone         = 0x00000000,
    InputMouseLeftButton   = 0x00000001,
    InputMouseRightButton  = 0x00000002,
    InputMouseMiddleButton = 0x00000004,
    InputMouseButton4      = 0x00000008,
    InputMouseButton5      = 0x00000010,
    // InputMouseWheelTiltLeft  = 0x00000020,
    // InputMouseWheelTiltRight = 0x00000040
} InputMouseButtonFlags;
typedef uint32_t InputMouseButtons;

struct InputKeyState {
    // uint32_t scan_code;
    // uint32_t code_point;
    SInputKeyCode virtual_key;
    bool          is_dead_key;
};

struct InputMouseState {
    InputMouseButtons buttons;
    int64_t           positionX;
    int64_t           positionY;
    int64_t           wheelX;
    int64_t           wheelY;
};

typedef enum EInputResult
{
    INPUT_RESULT_OK,
    INPUT_RESULT_FAIL,
    INPUT_RESULT_NOT_FOUND
} EInputResult;

using LayerId                            = skr_guid_t;
static const LayerId kGameInputLayerId   = { 0xa0bb28b1, 0xacdb, 0x41fb, { 0x87, 0xaa, 0x9d, 0x09, 0xfb, 0x92, 0x31, 0x8f } };
static const LayerId kCommonInputLayerId = { 0x1b1487f5, 0x7850, 0x4b85, { 0x9f, 0xc3, 0x0a, 0x9f, 0x81, 0x28, 0xcc, 0x5a } };
struct SKR_INPUT_API InputLayer {
    virtual ~InputLayer() SKR_NOEXCEPT;

    virtual void     GetLayerId(LayerId* out_id) const SKR_NOEXCEPT = 0;
    virtual bool     Initialize() SKR_NOEXCEPT                      = 0;
    virtual bool     Finalize() SKR_NOEXCEPT                        = 0;
    virtual bool     SetEnabled(bool enabled) SKR_NOEXCEPT          = 0;
    virtual bool     IsEnabled() const SKR_NOEXCEPT                 = 0;
    virtual void     Tick() SKR_NOEXCEPT {}
    virtual uint64_t GetReadingHistoryLifetimeUSec() const SKR_NOEXCEPT = 0;

    virtual uint64_t     GetCurrentTimestampUSec() SKR_NOEXCEPT                                                                                     = 0;
    virtual EInputResult GetCurrentReading(EInputKind kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT                           = 0;
    virtual EInputResult GetNextReading(InputReading* reference, EInputKind kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT     = 0;
    virtual EInputResult GetPreviousReading(InputReading* reference, EInputKind kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT = 0;

    virtual void     GetDevice(InputReading* reading, InputDevice** out_device) SKR_NOEXCEPT                              = 0;
    virtual uint32_t GetKeyState(InputReading* reading, uint32_t stateArrayCount, InputKeyState* stateArray) SKR_NOEXCEPT = 0;
    virtual bool     GetMouseState(InputReading* reading, InputMouseState* state) SKR_NOEXCEPT                            = 0;
    virtual uint64_t GetTimestampUSec(InputReading* reading) SKR_NOEXCEPT                                                 = 0;

    virtual void Release(InputReading* reading) SKR_NOEXCEPT = 0;
    virtual void Release(InputDevice* device) SKR_NOEXCEPT   = 0;
};

struct SKR_INPUT_API Input {
    Input() SKR_NOEXCEPT;
    virtual ~Input() SKR_NOEXCEPT;

    static void   Initialize() SKR_NOEXCEPT;
    static void   Finalize() SKR_NOEXCEPT;
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
    skr::span<InputLayer*> GetLayers() SKR_NOEXCEPT;

protected:
    static Input* instance_;
};

} // namespace input
} // namespace skr