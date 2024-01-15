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

    // accept or reject
    void request_accept(CombinePointerId pointer);
    void request_reject(CombinePointerId pointer);
    void request_accept_all();
    void request_reject_all();

    // clean
    inline void clean_tracing_pointers() { _tracing_pointers.clear(); }

private:
    Map<CombinePointerId, GestureArena*> _tracing_pointers;
};

} // namespace gui sreflect
} // namespace skr sreflect
