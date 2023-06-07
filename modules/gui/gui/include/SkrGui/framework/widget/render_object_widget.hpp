#pragma once
#include "SkrGui/framework/widget/widget.hpp"

namespace skr::gui
{
struct RenderObject;
struct BuildContext;

struct SKR_GUI_API RenderObjectWidget : public Widget {
    SKR_GUI_TYPE(RenderObjectWidget, "6c7edceb-7e04-446d-9a6a-f43f8a68dd17", Widget);

    virtual not_null<RenderObject*> create_render_object() SKR_NOEXCEPT = 0;
    virtual void update_render_object(not_null<BuildContext*> context, not_null<RenderObject*> render_object) SKR_NOEXCEPT = 0;

    // call after render object detached from render object tree
    virtual void did_update_render_object(not_null<RenderObject*> render_object) SKR_NOEXCEPT = 0;
};
} // namespace skr::gui