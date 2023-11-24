#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/matrix.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/event.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
// Event Route 的作用被其它机制所替代，是否可以直接干掉 Route 概念
sreflect_enum_class("guid": "51ed008c-6f63-495c-b229-204986d45cd5")
EEventRoutePhase : uint32_t
{
    None        = 0,
    TrickleDown = 1 << 0, // 预览事件，WPF 中一般用于 BubbleUp 被阻断的情况，通过 Notification 可能可以代替掉这一功能
    Reach       = 1 << 1, // 事件由确定的目标引发，典型的如 Click 事件
    Broadcast   = 1 << 2, // 一般没什么用
    BubbleUp    = 1 << 3, // 常用的处理事件方式
    All         = TrickleDown | Reach | Broadcast | BubbleUp,
    NoBroadcast = TrickleDown | Reach | BubbleUp,
};

sreflect_enum_class(
    "guid" : "03ff08f9-ba01-465a-991c-a6cfa294ddc4"
)
EEventSource : uint32_t
{
    None = 0,
    Device,
    Gesture,
    Framework,
};

sreflect_struct(
    "guid": "06ecf250-43e8-44a3-b1e9-b52b1ab53e05"
)
Event : virtual public skr::rttr::IObject {
    SKR_RTTR_GENERATE_BODY()
    EEventRoutePhase phase  = EEventRoutePhase::None;
    EEventSource     source = EEventSource::None;
};
} // namespace gui sreflect
} // namespace skr sreflect