#include "SkrGui/framework/render_box.hpp"

namespace skr
{
namespace gui
{

// Defines the direction in which the flex container's children are laid out.
enum class FlexDirection
{
    Row,          // Children are laid out horizontally from left to right.
    RowReverse,   // Children are laid out horizontally from right to left.
    Column,       // Children are laid out vertically from top to bottom.
    ColumnReverse // Children are laid out vertically from bottom to top.
};

// Defines how the children are distributed along the main axis of the flex container.
enum class JustifyContent
{
    FlexStart,    // Children are packed at the start of the main axis.
    FlexEnd,      // Children are packed at the end of the main axis.
    Center,       // Children are centered along the main axis.
    SpaceBetween, // Children are evenly distributed with the first child at the start and the last child at the end.
    SpaceAround,  // Children are evenly distributed with equal space around them.
    SpaceEvenly   // Children are evenly distributed with equal space between them.
};

// Defines how the children are aligned along the cross axis of the flex container.
enum class AlignItems
{
    FlexStart, // Children are aligned at the start of the cross axis.
    FlexEnd,   // Children are aligned at the end of the cross axis.
    Center,    // Children are centered along the cross axis.
    Stretch,   // Children are stretched to fill the cross axis.
    Baseline   // Children are aligned based on their baseline.
};

enum class FlexFit
{
    Tight,
    Loose,
};

struct Flexable {
    float flex = 1;     // determines how much the child should grow or shrink relative to other flex items
    FlexFit flex_fit = FlexFit::Loose; // determines how much the child should be allowed to shrink relative to its own size
};

class SKR_GUI_API RenderFlex : public RenderBox
{
public:
    RenderFlex(skr_gdi_device_id gdi_device);

    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    Flexable get_flex(int index); //each child's corresponding flexable property
    virtual void add_child(RenderElement* child) override;
    virtual void insert_child(RenderElement* child, int index) override;
    virtual void remove_child(RenderElement* child) override;
    void set_flexable(int index, Flexable flexable);

    void set_justify_content(JustifyContent justify_content);
    void set_flex_direction(FlexDirection flex_direction);
    void set_align_items(AlignItems align_items);
    JustifyContent get_justify_content();
    FlexDirection get_flex_direction();
    AlignItems get_align_items();

private:
    JustifyContent justify_content = JustifyContent::FlexStart;
    FlexDirection flex_direction = FlexDirection::Row;
    AlignItems align_items = AlignItems::FlexStart;
    VectorStorage<Flexable> flexables;
};


} // namespace gui
} // namespace skr