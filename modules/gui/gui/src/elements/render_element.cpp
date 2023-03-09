#include "SkrGui/render_elements/element.hpp"

namespace skr
{
namespace gui
{
    RenderElement::RenderElement()
    {
    }
    RenderElement::~RenderElement()
    {
    }
    void RenderElement::set_parent(RenderElement* parent)
    {
        this->parent = parent;
    }
    void RenderElement::add_child(RenderElement* child)
    {
        children.push_back(child);
    }
    void RenderElement::insert_child(RenderElement* child, int index)
    {
        children.insert(children.begin() + index, child);
    }
    int RenderElement::get_child_index(RenderElement* child)
    {
        for (int i = 0; i < children.size(); ++i)
        {
            if (children[i] == child)
            {
                return i;
            }
        }
        return -1;
    }
    void RenderElement::remove_child(RenderElement* child)
    {
        for (auto it = children.begin(); it != children.end(); ++it)
        {
            if (*it == child)
            {
                children.erase(it);
                break;
            }
        }
    }
    void RenderElement::set_render_matrix(const rtm::matrix4x4f& matrix)
    {
        render_matrix = matrix;
    }
    void RenderElement::set_active(bool active)
    {
        this->active = active;
    }
    void RenderElement::markLayoutDirty()
    {
        layoutDirty = true;
    }
} // namespace gui
} // namespace skr