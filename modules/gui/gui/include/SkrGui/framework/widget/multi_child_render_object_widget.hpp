#pragma once
#include "SkrGui/framework/widget/render_object_widget.hpp"

namespace skr::gui
{
struct SKR_GUI_API MultiChildRenderObjectWidget : public RenderObjectWidget {
    SKR_GUI_TYPE(MultiChildRenderObjectWidget, "d9e3dde8-f6e2-48bc-8a33-13f109ea5149", RenderObjectWidget);

    const Array<Widget*>& children() const SKR_NOEXCEPT { return _children; }

protected:
    Array<Widget*> _children;
};
} // namespace skr::gui