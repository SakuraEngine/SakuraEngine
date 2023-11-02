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
sreflect_enum_class(
    "guid": "51ed008c-6f63-495c-b229-204986d45cd5"
)
EEventRoutePhase : int32_t
{
    None        = 0,
    TrickleDown = 1 << 0,
    Reach       = 1 << 1,
    Broadcast   = 1 << 2,
    BubbleUp    = 1 << 3,
    All         = TrickleDown | Reach | Broadcast | BubbleUp,
    NoBroadcast = TrickleDown | Reach | BubbleUp,
};

sreflect_enum_class(
    "guid" : "03ff08f9-ba01-465a-991c-a6cfa294ddc4"
)
EEventSource : int32_t
{
    None = 0,
    Device,
    Gesture,
    Framework,
};

sreflect_struct(
    "guid": "06ecf250-43e8-44a3-b1e9-b52b1ab53e05",
    "rtti": true
)
Event : virtual public skr::rttr::IObject {
    EEventRoutePhase phase  = EEventRoutePhase::None;
    EEventSource     source = EEventSource::None;
};
} // namespace gui sreflect
} // namespace skr sreflect