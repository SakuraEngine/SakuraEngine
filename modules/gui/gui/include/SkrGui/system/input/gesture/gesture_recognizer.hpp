#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/system/input/pointer_event.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/gesture/gesture_recognizer.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct InputManager;

// 关于手势优先级
// Click 事件一般只接受被动触发
// Double Click 事件会在第一次 Click 之后延长判定时长，并在第二次 click 后主动触发
// Drag 事件会在抵达开始拖拽的距离之后主动触发
// 如此 Drag 会最先主动触发，随后是 Double Click，Click 永远以被动姿态触发
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
    inline static size_t _skr_hash(const CombinePointerId& id) SKR_NOEXCEPT
    {
        return skr::hash_combine(id.pointer_id, id.button_id);
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

    GestureRecognizer(NotNull<InputManager*> manager);

    // 监听 pointer
    virtual void add_pointer(NotNull<Event*> event) = 0;

    // 事件处理
    virtual bool handle_event(Event* event)             = 0;
    virtual bool handle_event_from_widget(Event* event) = 0;

    // 手势竞争
    virtual void accept_gesture(CombinePointerId pointer) = 0;
    virtual void reject_gesture(CombinePointerId pointer) = 0;

    // getter
    inline InputManager* input_manager() const { return _input_manager; }

private:
    InputManager* _input_manager = nullptr;
};

} // namespace gui sreflect
} // namespace skr sreflect