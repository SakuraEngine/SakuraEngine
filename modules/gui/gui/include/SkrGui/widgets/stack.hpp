#pragma once
#include "SkrGui/framework/widget/multi_child_render_object_widget.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr::gui
{
struct SKR_GUI_API Stack : public MultiChildRenderObjectWidget {
    SKR_GUI_TYPE(Stack, "94714676-422c-4e4e-a754-e26b5466900f", MultiChildRenderObjectWidget)

    Alignment      stack_alignment = Alignment::TopLeft();
    EPositionalFit child_fit = EPositionalFit::PassThrough;
    EStackSize     stack_size = EStackSize::Shrink;
    Array<Widget*> stack_children;
};
} // namespace skr::gui