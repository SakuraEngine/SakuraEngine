#pragma once
#include "SkrGui/framework/widget/multi_child_render_object_widget.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr::gui
{
struct SKR_GUI_API Stack : public MultiChildRenderObjectWidget {
    SKR_GUI_TYPE(Stack, "94714676-422c-4e4e-a754-e26b5466900f", MultiChildRenderObjectWidget)

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    Alignment      stack_alignment = Alignment::TopLeft();
    EPositionalFit child_fit = EPositionalFit::PassThrough;
    EStackSize     stack_size = EStackSize::Shrink;
};
} // namespace skr::gui