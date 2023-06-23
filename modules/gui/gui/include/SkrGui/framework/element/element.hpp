#pragma once
#include "SkrGui/framework/diagnostics.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/framework/build_context.hpp"
#include "SkrGui/framework/slot.hpp"

namespace skr::gui
{
enum class EElementLifecycle : uint8_t
{
    Initial,   // 初始化
    Mounted,   // 完全状态 (被挂在在树上)
    Unmounted, // 准备销毁前的缓冲状态 (从树上解绑)，有概率被复用
    Destroyed, // 完全被销毁
};

struct SKR_GUI_API Element : public DiagnosticableTreeNode, public IBuildContext {
    SKR_GUI_TYPE(Element, "123127c7-4eed-4007-87ff-6843bd56771a", DiagnosticableTreeNode, IBuildContext)
    using VisitFuncRef = FunctionRef<void(NotNull<Element*>)>;

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
    virtual void perform_rebuild() SKR_NOEXCEPT = 0;
    virtual void update_slot(Slot new_slot) SKR_NOEXCEPT; // TODO. move to FUCK RenderObjectElement
    virtual void update(NotNull<Widget*> new_widget) SKR_NOEXCEPT;

    // render object (self or child's)
    // TODO. move to FUCK RenderObjectElement
    virtual RenderObject* render_object() const SKR_NOEXCEPT;

    // TODO. notification
    // TODO. IBuildContext API

    // getter & setter
    inline Element* parent() const SKR_NOEXCEPT { return _parent; }
    inline Slot     slot() const SKR_NOEXCEPT { return _slot; }
    inline Widget*  widget() const SKR_NOEXCEPT { return _widget; }

protected:
    // help functions
    inline void       set_owner(BuildOwner* owner) SKR_NOEXCEPT { _owner = owner; }
    Element*          _update_child(Element* child, Widget* new_widget, Slot new_slot) SKR_NOEXCEPT;
    NotNull<Element*> _inflate_widget(NotNull<Widget*> widget, Slot slot) SKR_NOEXCEPT;
    inline void       _cancel_dirty() SKR_NOEXCEPT { _dirty = false; }
    void              _deactivate_child(NotNull<Element*> child) SKR_NOEXCEPT;
    void              _activate_child_with_parent(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT;

private:
    friend struct BuildOwner;
    // element tree
    Element*    _parent = nullptr;
    BuildOwner* _owner = nullptr;
    uint32_t    _depth = 0;

    // dirty marks & lifecycle
    bool              _dirty = false;
    bool              _in_dirty_list = false;
    EElementLifecycle _lifecycle = EElementLifecycle::Initial;

    // context
    // TODO. InheritedElement 的广播 CowMap<TYPE_ID, InheritedElement> _inherited_elements;
    // TODO. InheritedElement 的广播 Set<InheritedElement> _dependencies;
    Slot    _slot;
    Widget* _widget = nullptr;
};
} // namespace skr::gui