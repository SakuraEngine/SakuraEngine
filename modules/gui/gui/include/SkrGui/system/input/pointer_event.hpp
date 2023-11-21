#pragma once
#include "SkrGui/system/input/event.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/pointer_event.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_enum_class(
    "guid": "7223961b-5309-4fac-8207-8476bf7f3b05"
)
EPointerDeviceType : int32_t
{
    Unknown = 0,
    Mouse,    // 鼠标，Down/Up/Hover+Scroll/Scale
    Touch,    // 触摸屏，Add/Remove/Move
    TrackPad, // 触摸板，[光标的功能]+Pan/Zoom
    // Stylus,
    // InvertedStylus,
};
sreflect_enum_class(
    "guid": "3f6c7998-a93e-424e-9d95-c75800309a16"
)
EPointerButton : int32_t
{
    Unknown = 0,
    Left,
    Right,
    Middle,
    X1B,
    X2B,
    X3B,
    X4B,
    X5B,
};

sreflect_struct(
    "guid": "fa4706c4-5982-4056-a5b5-b12098c0963a"
)
PointerEvent : public Event {
    SKR_RTTR_GENERATE_BODY()
    EPointerDeviceType device_type     = EPointerDeviceType::Unknown; // 来源设备
    Offsetf            global_position = {};                          // 位置，全局空间的逻辑坐标
    Offsetf            global_delta    = {};                          // 距离上一次触发的位置变化量
    EPointerButton     button          = EPointerButton::Unknown;     // 按键 id
    Matrix4            transform       = {};                          // global->local 转换矩阵
};

sreflect_struct(
    "guid": "91343bb8-4b20-4734-b491-23362b76aa17"
)
PointerDownEvent : public PointerEvent {
    SKR_RTTR_GENERATE_BODY()
};

sreflect_struct(
    "guid": "8817dd2b-7e0d-47f5-8ee5-72790bbf3f09"
)
PointerUpEvent : public PointerEvent {
    SKR_RTTR_GENERATE_BODY()
};

sreflect_struct(
    "guid": "453f7052-9740-4136-9831-55e8188827d2"
)
PointerMoveEvent : public PointerEvent {
    SKR_RTTR_GENERATE_BODY()
};

sreflect_struct(
    "guid": "6cddd04b-749c-4a5e-99b0-27396ef84d50"
)
PointerEnterEvent : public PointerEvent {
    SKR_RTTR_GENERATE_BODY()
};

sreflect_struct(
    "guid": "f41e2f74-b813-411d-b065-5df10f5edaeb"
)
PointerExitEvent : public PointerEvent {
    SKR_RTTR_GENERATE_BODY()
};

sreflect_struct(
    "guid": "51778097-47ab-4eb3-9193-3cecffedf8a0"
)
PointerSignalEvent : public PointerEvent {
    SKR_RTTR_GENERATE_BODY()
};

sreflect_struct(
    "guid": "1da2a830-544a-44c4-8ba7-2f313194bced"
)
PointerScrollEvent : public PointerEvent {
    SKR_RTTR_GENERATE_BODY()
    Offsetf scroll_delta = {};
};

} // namespace gui sreflect
} // namespace skr sreflect