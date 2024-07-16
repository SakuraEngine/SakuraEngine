#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/matrix.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/event.generated.h"
#endif

namespace skr::gui
{
// Event Route 的作用被其它机制所替代，是否可以直接干掉 Route 概念
// 干不掉，根本干不掉，在移动端场景，手势处理需要跨越多个层级收集，而后竞争，这代表单纯的靠可阻断的 Bubble 无法达成功能
// 但是，这也是分平台的，比如 Desktop 的手势收集直接在 Bubble 阶段接受 Block 就行了
// 而 Touch 的手势收集则需要在 Broadcast 阶段执行，因为手势功能经常由多个层级提供
sreflect_enum_class("guid": "51ed008c-6f63-495c-b229-204986d45cd5")
EEventRoutePhase : uint32_t
{
    None        = 0,
    TrickleDown = 1 << 0, // 预览事件，WPF 中一般用于 BubbleUp 被阻断的情况，通过 Notification 或许可以代替掉这一功能
    Reach       = 1 << 1, // 事件由确定的目标引发，典型的如 Click 事件
    Broadcast   = 1 << 2, // 在 flutter 中通过类似的方式来实现手势的注册，在 Touch 设备下，我们也会使用这个模式
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
    SKR_GENERATE_BODY()
    EEventRoutePhase phase  = EEventRoutePhase::None;
    EEventSource     source = EEventSource::None;
};
} // namespace skr::gui