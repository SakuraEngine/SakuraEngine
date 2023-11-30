#pragma once
#include "SkrGui/system/input/gesture/gesture_recognizer.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/gesture/pointer_gesture_recognizer.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct GestureArena;

sreflect_struct("guid": "e4db9750-96ba-4558-8a14-402d1f9a2c7a")
SKR_GUI_API PointerGestureRecognizer : public GestureRecognizer {
    SKR_RTTR_GENERATE_BODY()
    using Super = GestureRecognizer;
    using Super::Super;

    // 监听 pointer
    void add_pointer(NotNull<Event*> event) override;

    // event
    virtual void on_pointer_added(PointerDownEvent* event) = 0;

private:
    UMap<CombinePointerId, GestureArena*> _tracing_pointers;
};

} // namespace gui sreflect
} // namespace skr sreflect
