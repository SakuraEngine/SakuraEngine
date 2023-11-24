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
//  1. Pointer 手势，用于 desktop，通常只有 Click、Double Click、Drag-Drop 等，可以实现一些特殊操作，
//     比如 houdini 的甩动节点等，desktop 的手势并不复杂，但是按键功能组合复杂
//  2. Touch 手势，相比于 Pointer 手势更复杂，用于移动端，不一一展开
//
// 但是，出于某些手势的复用性考虑，这里不进行继承结构上的区分
sreflect_struct("guid": "8fb085fd-9412-4a1b-bc95-a518e32746f2")
SKR_GUI_API GestureRecognizer : public skr::rttr::IObject {
    SKR_RTTR_GENERATE_BODY()

    bool handle_event(Event* event);

    // desktop pointer event 需要考虑的点
    //  1. 触发并追踪的 MouseButton
    //  2. 手势一经发出就不被终止
    //  3. 手势反过来对输入系统追踪的影响，比如 click 按下时，可能需要关闭 hover 事件的派发并主动派送 exit 事件来关闭 hover 效果
    //  4. 手势与发出者控件的交互，比如 click 要求 down/up 的对象是同一个控件
};

} // namespace gui sreflect
} // namespace skr sreflect