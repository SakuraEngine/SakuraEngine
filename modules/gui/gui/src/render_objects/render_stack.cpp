#include "SkrGui/render_objects/render_stack.hpp"
#include "misc/log.h"

namespace skr
{
namespace gui
{

RenderStack::RenderStack(gdi::IGDIDevice* gdi_device)
    : RenderBox(gdi_device)
{
    diagnostic_builder.add_properties(
    SkrNew<TextDiagnosticProperty>(u8"type", u8"stack", u8"place children in stack"));
}

void RenderStack::layout(BoxConstraint constraints, bool needSize)
{
    set_size(constraints.max_size());
    float width = get_size().width;
    float height = get_size().height;
    for (int i = 0; i < get_child_count(); i++)
    {
        RenderBox*    child = get_child_as_box(i);
        Positional    positional = get_position(i);
        BoxConstraint childConstraints = {};
        if (positional.left && positional.right)
        {
            if (positional.min_width || positional.max_width)
            {
                SKR_GUI_LOG_WARN("Both left and right are set, width will be ignored");
            }
            childConstraints.min_width = childConstraints.max_width = width - positional.left.get_pixel(width) - positional.right.get_pixel(width);
        }
        else
        {
            if (positional.min_width)
            {
                childConstraints.min_width = positional.min_width.get_pixel(width);
            }
            if (positional.max_width)
            {
                childConstraints.max_width = positional.max_width.get_pixel(width);
            }
        }
        if (positional.top && positional.bottom)
        {
            if (positional.min_height || positional.max_height)
            {
                SKR_GUI_LOG_WARN("Both top and bottom are set, height will be ignored");
            }
            childConstraints.min_width = childConstraints.max_width = height - positional.top.get_pixel(height) - positional.bottom.get_pixel(height);
        }
        else
        {
            if (positional.min_height)
            {
                childConstraints.min_height = positional.min_height.get_pixel(height);
            }
            if (positional.max_height)
            {
                childConstraints.max_height = positional.max_height.get_pixel(height);
            }
        }

        child->layout(childConstraints, true);
        Offset childPosition = { 0, 0 };
        if (positional.left)
        {
            float pivotX = positional.right ? 0 : positional.pivot.x;
            childPosition.x = positional.left.get_pixel(width) - child->get_size().width * pivotX;
        }
        else if (positional.right)
        {
            float pivotY = positional.pivot.y;
            childPosition.x = width - positional.right.get_pixel(width) - child->get_size().width * (1 - pivotY);
        }
        else
        {
            SKR_GUI_LOG_WARN("Both left and right are not set, default to left 0");
            childPosition.x = -child->get_size().width * positional.pivot.x;
        }
        if (positional.top)
        {
            float pivotY = positional.bottom ? 0 : positional.pivot.y;
            childPosition.y = positional.top.get_pixel(height) - child->get_size().height * pivotY;
        }
        else if (positional.bottom)
        {
            float pivotY = positional.pivot.y;
            childPosition.y = height - positional.bottom.get_pixel(height) - child->get_size().height * (1 - pivotY);
        }
        else
        {
            SKR_GUI_LOG_WARN("Both top and bottom are not set, default to top 0");
            childPosition.y = -child->get_size().height * positional.pivot.y;
        }
        child->set_position(childPosition);
    }
}

Positional RenderStack::get_position(int index)
{
    SKR_GUI_ASSERT(index >= 0 && index < positionals.size());
    return positionals[index];
}

void RenderStack::add_child(RenderObject* child)
{
    RenderBox::add_child(child);
    positionals.emplace_back(Positional{});
}

void RenderStack::insert_child(RenderObject* child, int index)
{
    RenderBox::insert_child(child, index);
    positionals.insert(positionals.begin() + index, Positional{});
}

void RenderStack::remove_child(RenderObject* child)
{
    positionals.erase(positionals.begin() + get_child_index(child));
    RenderBox::remove_child(child);
}

void RenderStack::set_positional(int index, Positional positional)
{
    SKR_GUI_ASSERT(index >= 0 && index < positionals.size());
    this->positionals[index] = positional;
}

} // namespace gui
} // namespace skr