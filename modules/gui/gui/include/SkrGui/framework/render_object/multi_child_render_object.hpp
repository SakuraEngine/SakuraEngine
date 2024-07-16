#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/framework/slot.hpp"
#include "SkrGui/framework/render_object/render_object.hpp"
#ifndef __meta__
    #include "SkrGui/framework/render_object/multi_child_render_object.generated.h"
#endif

namespace skr::gui
{
sreflect_interface(
    "guid": "409eaa24-5549-46e3-87c1-81649576d2cd"
)
SKR_GUI_API IMultiChildRenderObject : virtual public skr::rttr::IObject {
    SKR_GENERATE_BODY()
    virtual ~IMultiChildRenderObject() = default;

    virtual GUID accept_child_type() const SKR_NOEXCEPT                                    = 0;
    virtual void add_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT           = 0;
    virtual void remove_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT        = 0;
    virtual void move_child(NotNull<RenderObject*> child, Slot from, Slot to) SKR_NOEXCEPT = 0;
    virtual void flush_updates() SKR_NOEXCEPT                                              = 0;
};

template <typename TChild, typename TSlotData>
struct SlotStorage {
    // slot data
    TSlotData data = {};

    // child data
    Slot    desired_slot = {}; // used for slot update
    TChild* child        = nullptr;

    inline SlotStorage() SKR_NOEXCEPT = default;
    inline SlotStorage(Slot slot, TChild* child) SKR_NOEXCEPT : desired_slot(slot), child(child) {}
};

template <typename TChild>
struct SlotStorage<TChild, void> {
    Slot    desired_slot = {}; // used for slot update
    TChild* child        = nullptr;

    inline SlotStorage() SKR_NOEXCEPT = default;
    inline SlotStorage(Slot slot, TChild* child) SKR_NOEXCEPT : desired_slot(slot), child(child) {}
};

template <typename TSelf, typename TChild, typename TSlotData>
struct MultiChildRenderObjectMixin {
    Array<SlotStorage<TChild, TSlotData>> _children;
    bool                                  _need_flush_updates = false;

    inline GUID accept_child_type(const TSelf& self) const SKR_NOEXCEPT
    {
        return ::skr::rttr::type_id_of<TChild>();
    }
    inline void add_child(TSelf& self, NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT
    {
        _children.emplace(slot, child->type_cast_fast<TChild>());
        child->mount(&self);
        _need_flush_updates = true;
    }
    inline void remove_child(TSelf& self, NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT
    {
        // TODO. 使用 remove swap，保持 desired_slot 但是更新 child 的 slot
        auto& child_slot = _children[child->slot().index];
        if (child_slot.desired_slot != slot) { SKR_GUI_LOG_ERROR(u8"slot miss match when remove child"); }
        child_slot.child->unmount();
        child_slot.child    = nullptr;
        _need_flush_updates = true;
    }
    inline void move_child(TSelf& self, NotNull<RenderObject*> child, Slot from, Slot to) SKR_NOEXCEPT
    {
        _children[child->slot().index].desired_slot = to;
        _need_flush_updates                         = true;
    }
    inline void flush_updates(TSelf& self) SKR_NOEXCEPT
    {
        if (_need_flush_updates)
        {
            // step 1: remove all null slot
            for (size_t i = 0; i < _children.size();)
            {
                if (!_children[i].child)
                {
                    _children.remove_at_swap(i);
                }
                else
                {
                    ++i;
                }
            }

            // step 2: resort slot
            _sort_slots();

            // step 3: assign and check slot index
            for (size_t i = 0; i < _children.size(); ++i)
            {
                const auto& slot = _children[i];
                if (slot.desired_slot.index != i)
                {
                    SKR_GUI_LOG_ERROR(u8"slot index miss match");
                }
                slot.child->set_slot(slot.desired_slot);
            }
        }

        _need_flush_updates = false;
    }

    inline void _sort_slots()
    {
        _children.sort(
        [](const auto& lhs, const auto& rhs) {
            return lhs.desired_slot.index < rhs.desired_slot.index;
        });
    }
    inline void visit_children(const TSelf& self, RenderObject::VisitFuncRef visitor) const SKR_NOEXCEPT
    {
        for (const auto& slot : _children)
        {
            if (slot.child)
            {
                if (!visitor(slot.child))
                {
                    return;
                }
            }
        }
    }
};
} // namespace skr::gui

#define MULTI_CHILD_RENDER_OBJECT_MIX_IN(__SELF, __CHILD, __SLOT_DATA)                      \
    /*===============> Begin Multi Child Render Object Mixin <===============*/             \
private:                                                                                    \
    MultiChildRenderObjectMixin<__SELF, __CHILD, __SLOT_DATA>                               \
    _multi_child_render_object_mix_in = {};                                                 \
                                                                                            \
public:                                                                                     \
    GUID accept_child_type() const SKR_NOEXCEPT override                                    \
    {                                                                                       \
        return _multi_child_render_object_mix_in.accept_child_type(*this);                  \
    }                                                                                       \
    void add_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT override           \
    {                                                                                       \
        _multi_child_render_object_mix_in.add_child(*this, child, slot);                    \
    }                                                                                       \
    void remove_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT override        \
    {                                                                                       \
        _multi_child_render_object_mix_in.remove_child(*this, child, slot);                 \
    }                                                                                       \
    void move_child(NotNull<RenderObject*> child, Slot from, Slot to) SKR_NOEXCEPT override \
    {                                                                                       \
        _multi_child_render_object_mix_in.move_child(*this, child, from, to);               \
    }                                                                                       \
    void flush_updates() SKR_NOEXCEPT override                                              \
    {                                                                                       \
        _multi_child_render_object_mix_in.flush_updates(*this);                             \
    }                                                                                       \
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override                   \
    {                                                                                       \
        _multi_child_render_object_mix_in.visit_children(*this, visitor);                   \
    }                                                                                       \
    inline const Array<SlotStorage<__CHILD, __SLOT_DATA>>& children() const SKR_NOEXCEPT    \
    {                                                                                       \
        return _multi_child_render_object_mix_in._children;                                 \
    }                                                                                       \
    inline Array<SlotStorage<__CHILD, __SLOT_DATA>>& children() SKR_NOEXCEPT                \
    {                                                                                       \
        return _multi_child_render_object_mix_in._children;                                 \
    }                                                                                       \
    inline bool need_flush_updates() const SKR_NOEXCEPT                                     \
    {                                                                                       \
        return _multi_child_render_object_mix_in._need_flush_updates;                       \
    }                                                                                       \
    /*===============> End Multi Child Render Object Mixin <===============*/