#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/system/input/pointer_event.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/gesture_recognizer.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
// 手势区分为：
//  1. Pointer 手势，用于 desktop，通常只有 Click、Double Click、Drag-Drop 等，可以实现一些特殊操作，比如震动
//  2. Touch 手势，相比于 Pointer 手势更复杂，用于移动端，不一一展开
sreflect_struct("guid": "8fb085fd-9412-4a1b-bc95-a518e32746f2")
SKR_GUI_API GestureRecognizer : public skr::rttr::IObject {
    SKR_RTTR_GENERATE_BODY()
};

sreflect_struct("guid": "7ac4136e-e753-475c-98f2-9b7e44964000")
SKR_GUI_API PointerGestureRecognizer : public GestureRecognizer {
    SKR_RTTR_GENERATE_BODY()
    EPointerDeviceType support_device_type;
    void               add_pointer_pan_zoom(PointerPanZoomStartEvent* event);
    void               add_pointer(PointerDownEvent* event);
};

sreflect_struct("guid": "8bf39324-45f2-4341-afef-96f276526e88")
SKR_GUI_API TouchGestureRecognizer : public GestureRecognizer {
    SKR_RTTR_GENERATE_BODY()
    // TODO. SupportDevice
};

} // namespace gui sreflect
} // namespace skr sreflect