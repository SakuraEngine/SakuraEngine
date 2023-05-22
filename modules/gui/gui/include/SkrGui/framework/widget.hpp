#pragma once
#include "SkrGui/framework/diagnostics.hpp"
#include "SkrGui/framework/key.hpp"
#include "containers/not_null.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gui, Element, skr_gui_element)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, Slot, skr_gui_slot)
namespace skr {
namespace gui {

struct SKR_GUI_API Widget : public DiagnosticableTreeNode
{
    SKR_GUI_TYPE(Widget, DiagnosticableTreeNode, u8"9f69910d-ba18-4ff4-bf5f-3966507c56ba");
    virtual not_null<Element*> create_element() SKR_NOEXCEPT = 0;
    static bool CanUpdate(not_null<Widget*> old_widget, not_null<Widget*> new_widget) SKR_NOEXCEPT;

    Key key;
};

} }