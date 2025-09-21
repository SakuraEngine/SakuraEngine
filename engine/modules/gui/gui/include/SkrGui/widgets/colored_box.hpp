#pragma once
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "SkrGui/math/color.hpp"
#include "SkrGui/system/input/hit_test.hpp"
#include "SkrGui/widgets/colored_box.generated.h"

namespace skr::gui
{
struct [[sattr(guid = "66b7a12f-520d-4591-a31d-100f12211b17"
)]] SKR_GUI_API ColoredBox : public SingleChildRenderObjectWidget
{
    SKR_GENERATE_BODY(ColoredBox)

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    Color color = {};
    EHitTestBehavior hit_test_behaviour = EHitTestBehavior::opaque;
};
} // namespace skr::gui