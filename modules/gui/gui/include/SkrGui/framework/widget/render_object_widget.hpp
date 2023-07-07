#pragma once
#include "SkrGui/framework/widget/widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API RenderObjectWidget : public Widget {
    SKR_GUI_OBJECT(RenderObjectWidget, "6c7edceb-7e04-446d-9a6a-f43f8a68dd17", Widget);

    virtual NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT                                                                      = 0;
    virtual void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT = 0;

    // call after render object detached from render object tree
    virtual void did_unmount_render_object(NotNull<RenderObject*> render_object) SKR_NOEXCEPT;
};
} // namespace skr::gui