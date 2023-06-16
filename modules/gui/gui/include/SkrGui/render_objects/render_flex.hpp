#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr
{
namespace gui
{
struct Flexable {
    float   flex = 1;                  // determines how much the child should grow or shrink relative to other flex items
    FlexFit flex_fit = FlexFit::Loose; // determines how much the child should be allowed to shrink relative to its own size
};

class SKR_GUI_API RenderFlex : public RenderBox
{
public:
    SKR_GUI_TYPE(RenderFlex, "d3987dfd-24d2-478a-910e-537f24c4bae7", RenderBox);
    RenderFlex(gdi::IGDIDevice* gdi_device);

    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    Flexable     get_flex(int index); // each child's corresponding flexable property
    virtual void add_child(RenderObject* child) override;
    virtual void insert_child(RenderObject* child, int index) override;
    virtual void remove_child(RenderObject* child) override;
    void         set_flexable(int index, Flexable flexable);

    void           set_justify_content(JustifyContent justify_content);
    void           set_flex_direction(FlexDirection flex_direction);
    void           set_align_items(AlignItems align_items);
    JustifyContent get_justify_content();
    FlexDirection  get_flex_direction();
    AlignItems     get_align_items();

private:
    JustifyContent  justify_content = JustifyContent::FlexStart;
    FlexDirection   flex_direction = FlexDirection::Row;
    AlignItems      align_items = AlignItems::FlexStart;
    Array<Flexable> flexables;
};

} // namespace gui
} // namespace skr