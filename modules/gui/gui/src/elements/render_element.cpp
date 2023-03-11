#include "SkrGui/framework/render_element.hpp"

namespace skr {
namespace gui {

RenderElement::RenderElement()
{
    diagnostic_builder.add_properties(
        SkrNew<BoolDiagnosticProperty>("active", active, "")
    );
}

RenderElement::~RenderElement()
{

}

void RenderElement::set_parent(RenderElement* new_parent)
{
    if (parent)
    {
        parent->remove_child(this);
        parent = nullptr;
    }
    new_parent->add_child(this);
}

void RenderElement::add_child(RenderElement* child)
{
    auto& _children = this->children.get();
    _children.push_back(child);
    child->parent = parent;
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
    if (auto property = diagnostic_builder.find_property("active"))
    {
        property->as<BoolDiagnosticProperty>().value = active;
    }
    this->active = active;
}

void RenderElement::markLayoutDirty()
{
    layoutDirty = true;
}

void RenderElement::draw(const DrawParams* params) 
{
    if (!active) { return; }
    auto& _children = this->children.get();
    for (auto& child : _children)
    {
        child->draw(params);
    }
}

LiteSpan<DiagnosticableTreeNode* const> RenderElement::get_diagnostics_children() const
{
    const eastl::vector<RenderElement*>& children_ = children.get();
    return { (DiagnosticableTreeNode* const*)children_.data(), children_.size() };
}

} // namespace gui
} // namespace skr