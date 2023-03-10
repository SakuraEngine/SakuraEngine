#include "SkrGui/render_elements/element.hpp"

namespace skr {
namespace gui {

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
    auto& _children = this->children.get();
    _children.push_back(child);
}

void RenderElement::insert_child(RenderElement* child, int index)
{
    auto& _children = this->children.get();
    _children.insert(_children.begin() + index, child);
}

int RenderElement::get_child_index(RenderElement* child)
{
    auto& _children = this->children.get();
    for (int i = 0; i < _children.size(); ++i)
    {
        if (_children[i] == child)
        {
            return i;
        }
    }
    return -1;
}

void RenderElement::remove_child(RenderElement* child)
{
    auto& _children = this->children.get();
    for (auto it = _children.begin(); it != _children.end(); ++it)
    {
        if (*it == child)
        {
            _children.erase(it);
            break;
        }
    }
}

void RenderElement::set_render_matrix(const skr_float4x4_t& matrix)
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

void RenderElement::draw(skr_gdi_viewport_id viewport, skr_gdi_canvas_id canvas) 
{
    if (!active) { return; }
    auto& _children = this->children.get();
    for (auto& child : _children)
    {
        child->draw(viewport, canvas);
    }
}

} // namespace gui
} // namespace skr