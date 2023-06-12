#pragma once
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"

namespace skr::gui
{
struct SKR_GUI_API SizedBox : public SingleChildRenderObjectWidget {
    SKR_GUI_TYPE(SizedBox, "26d686b6-06b3-4514-af3d-f1354b53b884", SingleChildRenderObjectWidget)

    //==> Begin Construct
    struct Params {
        using WidgetType = SizedBox;
        float   width = std::numeric_limits<float>::infinity();
        float   height = std::numeric_limits<float>::infinity();
        Widget* child;
    };
    inline void construct(Params param)
    {
        _width = param.width;
        _height = param.height;
        _child = param.child;
    }
    struct Expand {
        using WidgetType = SizedBox;
        Widget* child;
    };
    inline void construct(Expand param)
    {
        _width = std::numeric_limits<float>::infinity();
        _height = std::numeric_limits<float>::infinity();
        _child = param.child;
    }
    struct Shrink {
        using WidgetType = SizedBox;
        Widget* child;
    };
    inline void construct(Shrink param)
    {
        _width = 0;
        _height = 0;
        _child = param.child;
    }
    //==> End Construct

private:
    float _width = 0;
    float _height = 0;
};
} // namespace skr::gui