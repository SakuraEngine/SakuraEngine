#pragma once
#include "SkrGui/framework/diagnostics.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/framework/build_context.hpp"

namespace skr::gui
{
enum class EElementLifecycle : uint8_t
{
    Initial,
    Active,
    Inactive,
    Defunct,
};

struct SKR_GUI_API Element : public DiagnosticableTreeNode, public IBuildContext {
    SKR_GUI_TYPE(Element, "123127c7-4eed-4007-87ff-6843bd56771a", DiagnosticableTreeNode, IBuildContext)

    Element(Widget* widget) SKR_NOEXCEPT;

    // element tree
    virtual void flush_depth() SKR_NOEXCEPT;
    virtual void visit_children(FunctionRef<void(Element*)> visitor) const SKR_NOEXCEPT;
    virtual void visit_children_recursive(FunctionRef<void(Element*)> visitor) const SKR_NOEXCEPT;

    // life circle
    virtual void mount(Element* parent, uint64_t slot) SKR_NOEXCEPT;
    virtual void activate() SKR_NOEXCEPT;
    virtual void deactivate() SKR_NOEXCEPT;
    virtual void unmount() SKR_NOEXCEPT;

    // owner
    inline BuildOwner* owner() const SKR_NOEXCEPT { return _owner; }
    inline void        set_owner(BuildOwner* owner) SKR_NOEXCEPT { _owner = owner; }

    // mark functions
    virtual void mark_needs_build() SKR_NOEXCEPT;

    // build & update
    void         rebuild(bool force = false) SKR_NOEXCEPT;
    virtual void perform_rebuild() SKR_NOEXCEPT = 0;
    virtual void update_slot(uint64_t new_slot) SKR_NOEXCEPT;
    virtual void update(NotNull<Widget*> new_widget) SKR_NOEXCEPT;

    // render object (self or child's)
    virtual RenderObject* render_object() const SKR_NOEXCEPT;

    // TODO. notification
    // TODO. IBuildContext API

    // getter & setter
    inline Element* parent() const SKR_NOEXCEPT { return _parent; }
    inline uint64_t slot() const SKR_NOEXCEPT { return _slot; }
    inline Widget*  widget() const SKR_NOEXCEPT { return _widget; }

protected:
    // help functions
    Element*          _update_child(Element* child, Widget* new_widget, uint64_t new_slot) SKR_NOEXCEPT;
    NotNull<Element*> _inflate_widget(NotNull<Widget*> widget, uint64_t slot) SKR_NOEXCEPT;
    inline void       _cancel_dirty() SKR_NOEXCEPT { _dirty = false; }
    // TODO. static helper update_children

private:
    friend struct BuildOwner;
    // element tree
    Element*    _parent = nullptr;
    BuildOwner* _owner = nullptr;
    uint32_t    _depth = 0;

    // dirty marks & lifecycle
    bool              _dirty = false;
    bool              _in_dirty_list = false;
    EElementLifecycle _lifecycle_state = EElementLifecycle::Initial;

    // context
    // TODO. InheritedElement 的广播 CowMap<TYPE_ID, InheritedElement> _inherited_elements;
    // TODO. InheritedElement 的广播 Set<InheritedElement> _dependencies;
    uint64_t _slot = 0;
    Widget*  _widget = nullptr;
};
} // namespace skr::gui