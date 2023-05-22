#pragma once
#include "SkrGui/framework/diagnostics.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gui, Element, skr_gui_element)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, Slot, skr_gui_slot)
namespace skr {
namespace gui {

struct SKR_GUI_API Widget : public DiagnosticableTreeNode
{
    SKR_GUI_TYPE(Widget, DiagnosticableTreeNode, u8"9f69910d-ba18-4ff4-bf5f-3966507c56ba");
    virtual Element* create_element() SKR_NOEXCEPT;
    static bool CanUpdate(Widget* old_widget, Widget* new_widget) SKR_NOEXCEPT;
};

} }