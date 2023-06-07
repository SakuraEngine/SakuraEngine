#pragma once
#include "SkrGui/framework/diagnostics.hpp"
#include "SkrGui/framework/key.hpp"
#include "SkrGui/framework/widget_misc.hpp"
#include "containers/not_null.hpp"

namespace skr::gui
{
struct Element;

struct SKR_GUI_API Widget : public DiagnosticableTreeNode {
    SKR_GUI_TYPE(Widget, DiagnosticableTreeNode, u8"9f69910d-ba18-4ff4-bf5f-3966507c56ba");

    inline const Key& key() const SKR_NOEXCEPT { return _key; }

    // bind element
    virtual not_null<Element*> create_element() SKR_NOEXCEPT = 0;

    // help function
    static bool can_update(not_null<Widget*> old_widget, not_null<Widget*> new_widget) SKR_NOEXCEPT;

protected:
    Key _key;
};
} // namespace skr::gui