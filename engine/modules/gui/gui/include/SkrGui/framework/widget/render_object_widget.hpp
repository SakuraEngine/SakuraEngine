#pragma once
#include "SkrGui/framework/widget/widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/framework/widget/render_object_widget.generated.h"

namespace skr::gui
{
struct [[sattr(guid = "e20ccde7-3f42-4224-aee3-9f54c9077194"
)]] SKR_GUI_API RenderObjectWidget : public Widget
{
    SKR_GENERATE_BODY(RenderObjectWidget)

    virtual NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT = 0;
    virtual void update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT = 0;

    // call after render object detached from render object tree
    virtual void did_unmount_render_object(NotNull<RenderObject*> render_object) SKR_NOEXCEPT;
};
} // namespace skr::gui