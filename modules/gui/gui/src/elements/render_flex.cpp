#include "SkrGui/render_objects/render_flex.hpp"

namespace skr
{
namespace gui
{

RenderFlex::RenderFlex(skr_gdi_device_id gdi_device)
    : RenderBox(gdi_device)
{
    diagnostic_builder.add_properties(
        SkrNew<TextDiagnosticProperty>("type", "flex", "layout children")
    );
}

void RenderFlex::layout(BoxConstraint constraints, bool needSize)
{
    // Determine main axis and cross axis based on flex direction
    skr_float2_t size = constraints.max_size;
    bool isRow = flex_direction == FlexDirection::Row ||
                 flex_direction == FlexDirection::RowReverse;
    float mainAxisSize = isRow ?
                         size.x :
                         size.y;
    float crossAxisSize = isRow ?
                          size.y :
                          size.x;

    // Calculate total flex factor of all children
    int totalFlex = 0;
    for (int i = 0; i < get_child_count(); i++)
    {
        totalFlex += get_flex(i).flex;
    }

    float availableSpace = mainAxisSize;
    // Calculate non-flexable children's size
    for (int i = 0; i < get_child_count(); i++)
    {
        RenderBox* child = get_child_as_box(i);
        Flexable flex = get_flex(i);
        if (flex.flex == 0)
        {
            child->layout(constraints, true);
            availableSpace -= isRow ?
                              child->get_size().x :
                              child->get_size().y;
        }
    }

    // Calculate size of each child based on flex factor and available space
    for (int i = 0; i < get_child_count(); i++)
    {
        RenderBox* child = get_child_as_box(i);
        Flexable flex = get_flex(i);
        BoxConstraint childConstraints = constraints;
        childConstraints.min_size = skr_float2_t{ 0, 0 };
        if (flex.flex > 0)
        {
            float childMainAxisSize = availableSpace * (float)flex.flex / (float)totalFlex;

            if (isRow)
            {
                childConstraints.max_size.x = childMainAxisSize;
            }
            else
            {
                childConstraints.max_size.y = childMainAxisSize;
            }
            if (align_items == AlignItems::Stretch)
            {
                if (isRow)
                {
                    childConstraints.min_size.y = crossAxisSize;
                }
                else
                {
                    childConstraints.min_size.x = crossAxisSize;
                }
            }
            if (flex.flex_fit == FlexFit::Tight)
            {
                if (isRow)
                {
                    childConstraints.min_size.x = childConstraints.max_size.x;
                }
                else
                {
                    childConstraints.min_size.y = childConstraints.max_size.y;
                }
            }
            child->layout(childConstraints, true);
        }
    }

    float mainAxisOffset = 0.0f;
    for (int i = 0; i < get_child_count(); i++)
    {
        RenderBox* child = get_child_as_box(i);
        skr_float2_t childSize = child->get_size();
        if (isRow)
        {
            mainAxisOffset += childSize.x;
        }
        else
        {
            mainAxisOffset += childSize.y;
        }
    }

    // Calculate position of each child based on justify content and align items
    float mainAxisStartPosition = 0.0f;
    if (justify_content == JustifyContent::FlexEnd)
    {
        mainAxisStartPosition = mainAxisSize - mainAxisOffset;
    }
    else if (justify_content == JustifyContent::Center)
    {
        mainAxisStartPosition = (mainAxisSize - mainAxisOffset) / 2.0f;
    }
    for (int i = 0; i < get_child_count(); i++)
    {
        if (justify_content == JustifyContent::SpaceBetween)
        {
            if (i > 0)
            {
                mainAxisStartPosition += (mainAxisSize - mainAxisOffset) / (get_child_count() - 1);
            }
        }
        else if (justify_content == JustifyContent::SpaceAround)
        {
            auto spacing = (mainAxisSize - mainAxisOffset) / get_child_count();
            if (i > 0)
            {
                mainAxisStartPosition += spacing;
            }
            else
            {
                mainAxisStartPosition += spacing * 0.5;
            }
        }
        else if (justify_content == JustifyContent::SpaceEvenly)
        {
            mainAxisStartPosition += (mainAxisSize - mainAxisOffset) / (get_child_count() + 1);
        }
        RenderBox* child = get_child_as_box(i);
        skr_float2_t childSize = child->get_size();
        float childMainAxisPosition = mainAxisStartPosition;
        float childCrossAxisPosition = 0.0f;
        if (align_items == AlignItems::FlexEnd ||
            align_items == AlignItems::Baseline)
        {
            childCrossAxisPosition = crossAxisSize - childSize.y;
        }
        else if (align_items == AlignItems::Center ||
                 align_items == AlignItems::Stretch)
        {
            childCrossAxisPosition = (crossAxisSize - childSize.y) / 2.0f;
        }
        if (flex_direction == FlexDirection::RowReverse ||
            flex_direction == FlexDirection::ColumnReverse)
        {
            childMainAxisPosition = mainAxisSize - childMainAxisPosition - childSize.x;
        }
        child->set_position({ childMainAxisPosition, childCrossAxisPosition });
        mainAxisStartPosition += childSize.x;
    }


    // Set size of flex container
    if (needSize)
    {
        float flexWidth = mainAxisSize;
        float flexHeight = crossAxisSize;
        if (flex_direction == FlexDirection::Column ||
            flex_direction == FlexDirection::ColumnReverse)
        {
            std::swap(flexWidth, flexHeight);
        }
        set_size({ flexWidth, flexHeight });
    }
}

void RenderFlex::add_child(RenderObject *child)
{
    RenderObject::add_child(child);
    flexables.get().emplace_back();
}

void RenderFlex::insert_child(RenderObject *child, int index)
{
    RenderObject::insert_child(child, index);
    flexables.get().insert(flexables.get().begin() + index, Flexable{});
}

void RenderFlex::remove_child(RenderObject *child)
{
    flexables.get().erase(flexables.get().begin() + get_child_index(child));
    RenderObject::remove_child(child);
}

void RenderFlex::set_flexable(int index, Flexable flexable)
{
    flexables.get()[index] = flexable;
}

Flexable RenderFlex::get_flex(int index)
{
    return flexables.get()[index];
}

void RenderFlex::set_flex_direction(FlexDirection direction)
{
    flex_direction = direction;
    markLayoutDirty();
}

FlexDirection RenderFlex::get_flex_direction()
{
    return flex_direction;
}

void RenderFlex::set_justify_content(JustifyContent content)
{
    justify_content = content;
    markLayoutDirty();
}

JustifyContent RenderFlex::get_justify_content()
{
    return justify_content;
}

void RenderFlex::set_align_items(AlignItems items)
{
    align_items = items;
    markLayoutDirty();
}

AlignItems RenderFlex::get_align_items()
{
    return align_items;
}

} // namespace gui
} // namespace skr