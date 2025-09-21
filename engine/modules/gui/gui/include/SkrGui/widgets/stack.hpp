#pragma once
#include "SkrGui/framework/widget/multi_child_render_object_widget.hpp"
#include "SkrGui/math/layout.hpp"
#include "SkrGui/widgets/stack.generated.h"

namespace skr::gui
{
struct [[sattr(guid = "e8c0541e-766d-4387-a56f-736bf9be4690"
)]] SKR_GUI_API Stack : public MultiChildRenderObjectWidget
{
    SKR_GENERATE_BODY(Stack)

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    Alignment stack_alignment = Alignment::TopLeft();
    EPositionalFit child_fit = EPositionalFit::PassThrough;
    EStackSize stack_size = EStackSize::Shrink;
};
} // namespace skr::gui