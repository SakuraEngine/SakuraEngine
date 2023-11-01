#pragma once
#include "SkrGui/system/input/event.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/pointer_event.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_enum(
    "guid": "7223961b-5309-4fac-8207-8476bf7f3b05",
    "rtti": true
)
EPointerDeviceType : int32_t
{
    Unknown = 0,
    Mouse,
    Touch,
    // Stylus,
    // InvertedStylus,
    // TrackPad,
};

sreflect_struct(
    "guid": "fa4706c4-5982-4056-a5b5-b12098c0963a",
    "rtti": true
)
PointerEvent : public Event {
    spush_attr("no-rtti": true)
    INativeWindow*     window          = nullptr;                     // 来源 window
    EPointerDeviceType device_type     = EPointerDeviceType::Unknown; // 来源设备
    Offsetf            global_position = {};                          // 位置，全局空间的逻辑坐标
    Offsetf            global_delta    = {};                          // 距离上一次触发的位置变化量
    int64_t            button_id       = 0;                           // 按键 id
    bool               is_down         = false;                       // 按键是否按下
    Matrix4            transform       = {};                          // global->local 转换矩阵
};
} // namespace gui sreflect
} // namespace skr sreflect