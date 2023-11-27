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
static const uint32_t kGesturePriorityClickOrTap = 1; // 最低判定优先级，因为容易阻断其它手势的判定
static const uint32_t kGesturePriorityLongPress  = 2;
static const uint32_t kGesturePriorityDrag       = 3;

struct CombinePointerId {
    uint32_t pointer_id = 0;
    uint32_t button_id  = 0;

    inline bool operator==(const CombinePointerId& other) const SKR_NOEXCEPT
    {
        return pointer_id == other.pointer_id && button_id == other.button_id;
    }
    inline bool operator!=(const CombinePointerId& other) const SKR_NOEXCEPT
    {
        return pointer_id != other.pointer_id || button_id != other.button_id;
    }
    inline size_t _skr_hash() const SKR_NOEXCEPT
    {
        return skr::hash_combine(pointer_id, button_id);
    }
};

// 手势区分为：
//  1. Pointer 手势，用于 desktop，通常只有 Click、Double Click、Drag-Drop 等，可以实现一些特殊操作，
//     比如 houdini 的甩动节点等，desktop 的手势并不复杂，但是按键功能组合复杂
//  2. Touch 手势，相比于 Pointer 手势更复杂，用于移动端，不一一展开
//
// 但是，出于某些手势的复用性考虑，这里不进行继承结构上的区分
sreflect_struct("guid": "8fb085fd-9412-4a1b-bc95-a518e32746f2")
SKR_GUI_API GestureRecognizer : public skr::rttr::IObject {
    SKR_RTTR_GENERATE_BODY()

    // 事件处理
    // TODO. 加上 TriggerPath 与 TriggerEvent
    virtual bool handle_event(Event* event);

    // 手势竞争
    virtual void accept_gesture(CombinePointerId pointer);
    virtual void reject_gesture(CombinePointerId pointer);

    uint32_t priority = 0;
};

} // namespace gui sreflect
} // namespace skr sreflect