#include "SkrGui/render_elements/render_stack.hpp"

namespace skr
{
namespace gui
{

RenderStack::RenderStack(skr_gdi_device_id gdi_device)
    : RenderBox(gdi_device)
{
}

void RenderStack::layout(BoxConstraint constraints, bool needSize)
{
    set_size(constraints.max_size);
    for (int i = 0; i < get_child_count(); i++)
    {
        RenderBox* child = get_child_as_box(i);
        Positional positional = get_position(i);
        BoxConstraint childConstraints;
        childConstraints.min_size = skr_float2_t{ 0, 0 };
        childConstraints.max_size = skr_float2_t{ std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
        if (positional.left && positional.right)
        {
            childConstraints.min_size.x = childConstraints.max_size.x =
            get_size().x - *positional.left - *positional.right;
        }
        else if(positional.width)
        {
            childConstraints.min_size.x = childConstraints.max_size.x = *positional.width;
        }
        if (positional.top && positional.bottom)
        {
            childConstraints.min_size.y = childConstraints.max_size.y =
            get_size().y - *positional.top - *positional.bottom;
        }
        else if (positional.height)
        {
            childConstraints.min_size.y = childConstraints.max_size.y = *positional.height;
        }

        child->layout(childConstraints, true);
        skr_float2_t childPosition = { 0, 0 };
        if(positional.left)
        {
            float pivotX = positional.right ? 0 : positional.pivot.x;
            childPosition.x = *positional.left - child->get_size().x * pivotX;
        }
        else if(positional.right)
        {
            float pivotY = positional.pivot.y;
            childPosition.x = get_size().x - *positional.right - child->get_size().x * (1 - pivotY);
        }
        else
        {
            childPosition.x = - child->get_size().x * positional.pivot.x;
        }
        if (positional.top)
        {
            float pivotY = positional.bottom ? 0 : positional.pivot.y;
            childPosition.y = *positional.top - child->get_size().y * pivotY;
        }
        else if (positional.bottom)
        {
            float pivotY = positional.pivot.y;
            childPosition.y = get_size().y - *positional.bottom - child->get_size().y * (1 - pivotY);
        }
        else
        {
            childPosition.y = -child->get_size().y * positional.pivot.y;
        }
        child->set_position(childPosition);
    }
}

Positional RenderStack::get_position(int index)
{
    assert(index >= 0 && index < positionals.get().size());
    return positionals.get()[index];
}

void RenderStack::add_child(RenderElement* child)
{
    RenderBox::add_child(child);
    positionals.get().emplace_back(Positional{});
}

void RenderStack::insert_child(RenderElement* child, int index)
{
    RenderBox::insert_child(child, index);
    positionals.get().insert(positionals.get().begin() + index, Positional{});
}

void RenderStack::remove_child(RenderElement* child)
{
    positionals.get().erase(positionals.get().begin() + get_child_index(child));
    RenderBox::remove_child(child);
}

void RenderStack::set_positional(int index, Positional positional)
{
    SKR_ASSERT(index >= 0 && index < positionals.get().size());
    this->positionals.get()[index] = positional;
}
} // namespace gui
} // namespace skr