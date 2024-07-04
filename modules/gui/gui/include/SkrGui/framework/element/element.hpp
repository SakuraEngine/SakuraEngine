#pragma once
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/framework/build_context.hpp"
#include "SkrGui/framework/slot.hpp"
#ifndef __meta__
    #include "SkrGui/framework/element/element.generated.h"
#endif

namespace skr::gui
{
enum class EElementLifecycle : uint8_t
{
    Initial,   // 初始化
    Mounted,   // 完全状态 (被挂在在树上)
    Unmounted, // 准备销毁前的缓冲状态 (从树上解绑)，有概率被复用
    Destroyed, // 完全被销毁
};

sreflect_struct(
    "guid": "ec3aff75-d8d1-4e0f-aff3-5c8df4995a78"
)
SKR_GUI_API Element : virtual public skr::rttr::IObject,
                      public IBuildContext {
    SKR_GENERATE_BODY()

    Element(Widget* widget) SKR_NOEXCEPT;

    // lifecycle & tree
    // ctor -> mount <-> unmount -> destroy
    void                     mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT;       // 挂到父节点下
    void                     unmount() SKR_NOEXCEPT;                                        // 从父节点卸载
    virtual void             destroy() SKR_NOEXCEPT;                                        // 销毁，生命周期结束
    virtual void             first_mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT; // 首次挂载到父节点下
    virtual void             attach(NotNull<BuildOwner*> owner) SKR_NOEXCEPT;               // 自身或子树挂载到带有 pipeline owner 的树下
    virtual void             detach() SKR_NOEXCEPT;                                         // 自身或子树从带有 pipeline owner 的树下卸载
    inline EElementLifecycle lifecycle() const SKR_NOEXCEPT { return _lifecycle; }
    inline BuildOwner*       owner() const SKR_NOEXCEPT { return _owner; }
    virtual void             visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT = 0;

    // mark functions
    virtual void mark_needs_build() SKR_NOEXCEPT;

    // build & update
    void         rebuild(bool force = false) SKR_NOEXCEPT;
    virtual void perform_rebuild() SKR_NOEXCEPT = 0;               // rebuild，由 BuildOwner 最先发起
    virtual void update(NotNull<Widget*> new_widget) SKR_NOEXCEPT; // widget 更换

    // TODO. notification
    // TODO. IBuildContext API

    //==> Begin IBuildContext API
    // data query
    Widget*     bound_widget() const SKR_NOEXCEPT override;
    BuildOwner* build_owner() const SKR_NOEXCEPT override;
    bool        is_destroyed() const SKR_NOEXCEPT override;

    // render object query
    RenderObject*   find_render_object() const SKR_NOEXCEPT override;
    Optional<Sizef> render_box_size() const SKR_NOEXCEPT override;

    // visit
    void visit_ancestor_elements(VisitFuncRef visitor) const SKR_NOEXCEPT override;
    void visit_child_elements(VisitFuncRef visitor) const SKR_NOEXCEPT override;
    //==> End IBuildContext API

    // getter & setter
    inline uint32_t depth() const SKR_NOEXCEPT { return _depth; }
    inline Element* parent() const SKR_NOEXCEPT { return _parent; }
    inline Slot     slot() const SKR_NOEXCEPT { return _slot; }
    inline Widget*  widget() const SKR_NOEXCEPT { return _widget; }
    inline bool     is_dirty() const SKR_NOEXCEPT { return _dirty; }

protected:
    // help functions
    Element*          _update_child(Element* child, Widget* new_widget, Slot new_slot) SKR_NOEXCEPT;
    void              _update_children(Array<Element*>& children, const Array<Widget*>& new_widgets);
    NotNull<Element*> _inflate_widget(NotNull<Widget*> widget, Slot slot) SKR_NOEXCEPT;
    void              _update_slot_for_child(NotNull<Element*> child, Slot new_slot) SKR_NOEXCEPT;
    void              _attach_render_object_children(Slot new_slot) SKR_NOEXCEPT;
    void              _detach_render_object_children() SKR_NOEXCEPT;

    inline void _cancel_dirty() SKR_NOEXCEPT { _dirty = false; }

    // for special mount progress
    friend struct RenderNativeWindowElement;

private:
    friend struct BuildOwner;
    // element tree
    Element*    _parent = nullptr;
    BuildOwner* _owner  = nullptr;
    uint32_t    _depth  = 0;

    // dirty marks & lifecycle
    bool              _dirty         = false;
    bool              _in_dirty_list = false;
    EElementLifecycle _lifecycle     = EElementLifecycle::Initial;

    // context
    // TODO. InheritedElement 的广播 CowMap<TYPE_ID, InheritedElement> _inherited_elements;
    // TODO. InheritedElement 的广播 Set<InheritedElement> _dependencies;
    Slot    _slot;
    Widget* _widget = nullptr;
};
} // namespace skr::gui