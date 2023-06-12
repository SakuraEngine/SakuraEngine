#pragma once
#include "SkrGui/framework/widget/multi_child_render_object_widget.hpp"
#include "SkrGui/framework/layout.hpp"

namespace skr::gui
{
struct SKR_GUI_API Flex : public MultiChildRenderObjectWidget {
    SKR_GUI_TYPE(Flex, "03fbfa97-39bb-4233-afdb-1f53648e5152", MultiChildRenderObjectWidget)
    struct Slot {
        float flex = 1;                    // determines how much the child should grow or shrink relative to other flex items
        FlexFit flex_fit = FlexFit::Loose; // determines how much the child should be allowed to shrink relative to its own size
        Widget* child;
    };

    //==> Begin Constructors
    struct Params {
        JustifyContent justify_content = JustifyContent::FlexStart;
        FlexDirection flex_direction = FlexDirection::Row;
        AlignItems align_items = AlignItems::FlexStart;
        Span<Slot> children;
    };
    void construct(const Params& params) SKR_NOEXCEPT;
    //==> End Constructors

private:
    JustifyContent _justify_content;
    FlexDirection _flex_direction;
    AlignItems _align_items;
    Array<Slot> _children_slots;
};

} // namespace skr::gui