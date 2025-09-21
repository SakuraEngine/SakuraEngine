#pragma once
#include "SkrGui/framework/widget/leaf_render_object_widget.hpp"
#include "SkrGui/widgets/text.generated.h"

namespace skr::gui
{
struct [[sattr(guid = "ba805fb4-6b04-4a41-9c7d-605b61d194aa"
)]] SKR_GUI_API Text : public LeafRenderObjectWidget
{
    SKR_GENERATE_BODY(Text)

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    String text = {};
};
} // namespace skr::gui