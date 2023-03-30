#pragma once
#include "SkrGui/framework/element.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gui, RenderObjectWidget, skr_gui_render_object_widget)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, SingleChildRenderObjectWidget, skr_gui_single_child_render_object_widget)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, MultiChildRenderObjectWidget, skr_gui_multi_child_render_object_widget)

namespace skr {
namespace gui {

struct SKR_GUI_API RenderObjectElement : public Element
{

};

struct SKR_GUI_API SingleChildRenderObjectElement : public RenderObjectElement
{

    LiteOptional<Element*> _child = nullptr;
};

struct SKR_GUI_API MultiChildRenderObjectElement : public RenderObjectElement
{

    VectorStorage<Element*> _children;
};

} // namespace gui
} // namespace skr