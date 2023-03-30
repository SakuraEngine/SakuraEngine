#pragma once
#include "utils/function_ref.hpp"
#include "SkrGui/framework/diagnostics.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gui, RenderObject, skr_gui_render_object)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, Widget, skr_gui_widget)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, Element, skr_gui_element)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, Slot, skr_gui_slot)

namespace skr {
namespace gui {

enum class ElementLifecycle : uint32_t
{
    initial,
    active,
    inactive,
    defunct,
};

struct SKR_GUI_API BuildContext : public DiagnosticableTreeNode
{
    virtual bool mounted() SKR_NOEXCEPT = 0;
    virtual Widget* get_widget() SKR_NOEXCEPT = 0;
    virtual LiteOptional<RenderObject*> find_render_object() SKR_NOEXCEPT = 0;
    virtual BoxSizeType get_size() SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API Element : public BuildContext
{
    Element(skr_gui_widget_id widget) SKR_NOEXCEPT;

    // element interfaces
    
    virtual void activate() SKR_NOEXCEPT;
    virtual void deactivate() SKR_NOEXCEPT;

    virtual void mount(LiteOptional<Element*> parent, LiteOptional<Slot*> slot) SKR_NOEXCEPT;
    virtual void unmount() SKR_NOEXCEPT;

    virtual void attach_render_object(LiteOptional<Slot*> new_slot) SKR_NOEXCEPT;
    virtual void detach_render_object() SKR_NOEXCEPT;

    virtual Element* inflate_widget(Widget* widget, LiteOptional<Slot*> new_slot) SKR_NOEXCEPT;

    virtual void update(Widget* new_widget) SKR_NOEXCEPT;
    virtual void update_slot_for_child(Element* child, LiteOptional<Slot*> new_slot) SKR_NOEXCEPT;
    virtual Element* update_child(LiteOptional<Element*> child, LiteOptional<Widget*> new_widget, LiteOptional<Slot*> new_slot) SKR_NOEXCEPT;
    virtual void visit_children(skr::function_ref<void(Element*)> visitor) SKR_NOEXCEPT;
    virtual void visit_child_elements(skr::function_ref<void(Element*)> visitor) SKR_NOEXCEPT;
    virtual void forget_child(Element* child) SKR_NOEXCEPT;

    virtual void perform_rebuild() SKR_NOEXCEPT;
    virtual void rebuild(bool force = false) SKR_NOEXCEPT;

    void _update_slot(LiteOptional<Slot*> new_slot) SKR_NOEXCEPT;

    // build context interfaces
    virtual bool mounted() SKR_NOEXCEPT override;
    virtual Widget* get_widget() SKR_NOEXCEPT override;
    virtual BoxSizeType get_size() SKR_NOEXCEPT override;
    virtual LiteOptional<RenderObject*> find_render_object() SKR_NOEXCEPT override;

    uint32_t _depth = 0;
    bool _dirty = true;

    skr_gui_widget_id _widget = nullptr;
    skr_gui_element_id _parent = nullptr;
    skr_gui_slot_id _slot = nullptr;
    ElementLifecycle _lifecycle_state = ElementLifecycle::initial;
};

} // namespace gui
} // namespace skr