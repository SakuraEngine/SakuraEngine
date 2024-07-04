#pragma once
#include "SkrGui/system/input/event.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/pointer_event.generated.h"
#endif

namespace skr::gui
{
sreflect_enum_class("guid": "7223961b-5309-4fac-8207-8476bf7f3b05")
EPointerDeviceType : uint32_t
{
    Unknown  = 0,
    Mouse    = 1 << 0, // 鼠标，Down/Up/Hover+Scroll/Scale
    TrackPad = 1 << 1, // 触摸板，[光标的功能]+Pan/Zoom

    // TODO. Touch 和 Stylus 应放到 TouchEvent 里面去
    // Touch,    // 触摸屏，Add/Remove/Move
    // Stylus,
    // InvertedStylus,
};
sreflect_enum_class("guid": "3f6c7998-a93e-424e-9d95-c75800309a16")
EPointerButton : uint32_t
{
    Unknown = 0,
    Left    = 1 << 0,
    Right   = 1 << 1,
    Middle  = 1 << 2,
    X1B     = 1 << 3,
    X2B     = 1 << 4,
    X3B     = 1 << 5,
    X4B     = 1 << 6,
    X5B     = 1 << 7,
};

sreflect_enum_class("guid": "f8f9a770-aa9a-4ce3-9ecf-613c8bda7c9a")
EPointerModifier : uint32_t
{
    Unknown      = 0,
    LeftCtrl     = 1 << 0, // mac  win
    RightCtrl    = 1 << 1, //      win
    LeftAlt      = 1 << 2, //      win
    RightAlt     = 1 << 3, //      win
    LeftShift    = 1 << 4, // mac  win
    RightShift   = 1 << 5, // mac  win
    LeftOption   = 1 << 6, // mac
    RightOption  = 1 << 7, // mac
    LeftCommand  = 1 << 8, // mac
    RightCommand = 1 << 9, // mac
};

// TODO. Touch Event, 与 PointerEvent 分开实现
sreflect_struct("guid": "fa4706c4-5982-4056-a5b5-b12098c0963a")
PointerEvent : public Event {
    SKR_GENERATE_BODY()

    EPointerDeviceType device_type     = EPointerDeviceType::Unknown; // 来源设备
    uint32_t           pointer_id      = 0;
    Offsetf            global_position = {};                      // 位置，全局空间的逻辑坐标
    Offsetf            global_delta    = {};                      // 距离上一次触发的位置变化量
    EPointerButton     button          = EPointerButton::Unknown; // 按键 id
    Matrix4            transform       = {};                      // global->local 转换矩阵

    // TODO. get mouse button states
    // TODO. get modifier key states
};

// Down/Up/Move
sreflect_struct("guid": "91343bb8-4b20-4734-b491-23362b76aa17")
PointerDownEvent : public PointerEvent {
    SKR_GENERATE_BODY()
};

sreflect_struct("guid": "8817dd2b-7e0d-47f5-8ee5-72790bbf3f09")
PointerUpEvent : public PointerEvent {
    SKR_GENERATE_BODY()
};

sreflect_struct("guid": "453f7052-9740-4136-9831-55e8188827d2")
PointerMoveEvent : public PointerEvent {
    SKR_GENERATE_BODY()
};

// Enter/Exit
sreflect_struct("guid": "6cddd04b-749c-4a5e-99b0-27396ef84d50")
PointerEnterEvent : public PointerEvent {
    SKR_GENERATE_BODY()
};

sreflect_struct("guid": "f41e2f74-b813-411d-b065-5df10f5edaeb")
PointerExitEvent : public PointerEvent {
    SKR_GENERATE_BODY()
};

// Scroll/Scale
sreflect_struct("guid": "1da2a830-544a-44c4-8ba7-2f313194bced")
PointerScrollEvent : public PointerEvent {
    SKR_GENERATE_BODY()
    Offsetf scroll_delta = {};
};
sreflect_struct("guid": "51778097-47ab-4eb3-9193-3cecffedf8a0")
PointerScaleEvent : public PointerEvent {
    SKR_GENERATE_BODY()
};

// Pan/Zoom
sreflect_struct("guid": "5d3aeff7-4cd5-41d2-93ba-0d0cdd14b9b0")
PointerPanZoomStartEvent : public PointerEvent {
    SKR_GENERATE_BODY()
};
sreflect_struct("guid": "f25e3d34-4fae-4be8-8f17-385b960a95f8")
PointerPanZoomUpdateEvent : public PointerEvent {
    SKR_GENERATE_BODY()
};
sreflect_struct("guid": "d8ee52fd-2f4e-4a62-a922-d3cb2467883b")
PointerPanZoomEndEvent : public PointerEvent {
    SKR_GENERATE_BODY()
};

} // namespace skr::gui