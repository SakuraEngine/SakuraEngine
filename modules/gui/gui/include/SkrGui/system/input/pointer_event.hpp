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
    IWindow*           window       = nullptr;                     // 来源 window
    uint64_t           time_stamp   = 0;                           // 时间戳 // TODO. 使用更文明的时间戳
    int64_t            pointer_id   = 0;                           // 指针 id
    EPointerDeviceType device_type  = EPointerDeviceType::Unknown; // 来源设备
    Offsetf            position     = {};                          // 位置，全局空间的逻辑坐标
    Offsetf            delta        = {};                          // 距离上一次触发的位置变化量
    int64_t            button_id    = 0;                           // 按键 id
    bool               is_down      = false;                       // 按键是否按下
    float              pressure     = 0;                           // 压力
    float              pressure_min = 0;                           // 压力最小值
    float              pressure_max = 0;                           // 压力最大值
    // float              distance     = 0;                           // 触摸设备距离表面的距离
    // float              distance_min = 0;                           // 触摸设备距离表面的最小距离
    // float              distance_max = 0;                           // 触摸设备距离表面的最大距离
    // float              size         = 0;                           // 触摸区域大小
    // float              radius_major = 0;                           // 触摸椭圆主轴半径
    // float              radius_minor = 0;                           // 触摸椭圆次轴半径
    // float              radius_min   = 0;                           // 触摸椭圆主/次轴最小半径
    // float              radius_max   = 0;                           // 触摸椭圆主/次轴最大半径
    // float              orientation  = 0;                           // 触摸笔笔尖方向
    // float              tilt         = 0;                           // 触摸笔笔尖角度
    Matrix4 transform = {}; // global->local 转换矩阵
};
} // namespace gui sreflect
} // namespace skr sreflect