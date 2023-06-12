#pragma once
#include "SkrGui/framework/widget/leaf_render_object_widget.hpp"

namespace skr::gui
{
struct SKR_GUI_API ColorPicker : public LeafRenderObjectWidget {
    SKR_GUI_TYPE(ColorPicker, "270fc1d7-19ab-4f1a-8c10-e08b69393425", LeafRenderObjectWidget)

    //==> Begin Constructor
    struct Params {
        using WidgetType = ColorPicker;
        bool is_srgb = false;
    };
    inline void construct(Params params) SKR_NOEXCEPT
    {
        _is_srgb = params.is_srgb;
    }
    //==> End Constructor

private:
    bool _is_srgb;
};
} // namespace skr::gui