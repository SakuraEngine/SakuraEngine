#pragma once
#include "SkrGui/framework/diagnostics.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
enum class ElementLifecycle : uint32_t
{
    initial,
    active,
    inactive,
    defunct,
};

struct SKR_GUI_API Element : public DiagnosticableTreeNode {
    SKR_GUI_TYPE(Element, "123127c7-4eed-4007-87ff-6843bd56771a", DiagnosticableTreeNode)

    friend struct BuildOwner;

    Element(Widget* widget) SKR_NOEXCEPT;

    // life circle
    virtual void activate() SKR_NOEXCEPT;
    virtual void deactivate() SKR_NOEXCEPT;

    // element tree modify
    virtual void mount(Element* parent, Slot* slot) SKR_NOEXCEPT;
    virtual void unmount() SKR_NOEXCEPT;

    // render object tree modify
    virtual void attach_render_object(Slot* new_slot) SKR_NOEXCEPT;
    virtual void detach_render_object() SKR_NOEXCEPT;

    // widget update
    // rebuild --> perform_rebuild --> update_child --> update_slot_for_child
    //                                              --> update
    //                                              --> inflate_widget
    void                      rebuild(bool force = false) SKR_NOEXCEPT;                                      // 控件树刷新的入口，由 BuildOwner 调用，实现固定，主要做一些 assert 工作
    virtual void              perform_rebuild() SKR_NOEXCEPT;                                                // 实际走到的 rebuild 逻辑，由具体的 Element 实现
    virtual Element*          update_child(Element* child, Widget* new_widget, Slot* new_slot) SKR_NOEXCEPT; // perform_rebuild 中调用，在这里做 child 的 diff 工作
    virtual void              update_slot_for_child(Element* child, Slot* new_slot) SKR_NOEXCEPT;            // 最低开销的更新，仅仅更新 slot
    virtual void              update(Widget* new_widget) SKR_NOEXCEPT;                                       // 更新 child 的数据，将 widget 信息透传到 render object
    virtual NotNull<Element*> inflate_widget(NotNull<Widget*> widget, Slot* new_slot) SKR_NOEXCEPT;          // 刷新 widget，最耗的更新

    // element tree query
    virtual void visit_children(function_ref<void(Element*)> visitor) SKR_NOEXCEPT;
    virtual void visit_child_elements(function_ref<void(Element*)> visitor) SKR_NOEXCEPT;
    virtual void forget_child(Element* child) SKR_NOEXCEPT;
    virtual void deactivate_child(Element* child) SKR_NOEXCEPT;

private:
    // help functions
    Element*     _retake_inactive_element(const Key& key, NotNull<Widget*> widget) SKR_NOEXCEPT;
    void         _active_with_parent(Element* parent, Slot* slot) SKR_NOEXCEPT;
    static void  _active_recursively(Element* element) SKR_NOEXCEPT;
    virtual void _update_slot(Slot* new_slot) SKR_NOEXCEPT;
    void         _update_depth(int parentDepth) SKR_NOEXCEPT;
    static int   _compare_depth(Element* a, Element* b) SKR_NOEXCEPT;
    bool         _debug_is_in_scope(Element* ancestor) SKR_NOEXCEPT;

private:
    uint32_t _depth = 0;
    bool     _dirty = true;
    bool     _in_dirty_list = false;

    Widget*          _widget = nullptr;
    Element*         _parent = nullptr;
    Slot*            _slot = nullptr; // TODO. use int64_t and rename to _slot_index, parentData -> slot and slot -> slot_index
    BuildOwner*      _owner = nullptr;
    ElementLifecycle _lifecycle_state = ElementLifecycle::initial;
};
} // namespace skr::gui