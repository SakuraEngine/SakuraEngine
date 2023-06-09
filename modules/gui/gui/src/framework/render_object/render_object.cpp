#include "SkrGui/framework/render_object/render_object.hpp"
#include "SkrGui/gdi/gdi.hpp"

namespace skr::gui
{

RenderObject::RenderObject()
{
    diagnostic_builder.add_properties(
        SkrNew<BoolDiagnosticProperty>(u8"active", active, u8""));
}

RenderObject::~RenderObject()
{
}

void RenderObject::set_parent(RenderObject* new_parent)
{
    if (parent)
    {
        parent->remove_child(this);
        parent = nullptr;
    }
    new_parent->add_child(this);
}

void RenderObject::add_child(RenderObject* child)
{
    children.push_back(child);
    child->parent = parent;
}

void RenderObject::insert_child(RenderObject* child, int index)
{
    children.insert(children.begin() + index, child);
}

int RenderObject::get_child_index(RenderObject* child)
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

void RenderObject::remove_child(RenderObject* child)
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

int RenderObject::get_child_count() const
{
    return children.size();
}

RenderObject* RenderObject::get_child(int index) const
{
    return children[index];
}

void RenderObject::set_render_matrix(const skr_float4x4_t& matrix)
{
    render_matrix = matrix;
}

void RenderObject::set_active(bool active)
{
    if (auto property = diagnostic_builder.find_property(u8"active"))
    {
        property->as<BoolDiagnosticProperty>().value = active;
    }
    this->active = active;
}

void RenderObject::markLayoutDirty()
{
    layoutDirty = true;
}

void RenderObject::before_draw(const DrawParams* params)
{
}

void RenderObject::draw(const DrawParams* params)
{
    if (!active) { return; }
    for (auto& child : children)
    {
        child->before_draw(params);
        child->draw(params);
        child->after_draw(params);
    }
}

void RenderObject::after_draw(const DrawParams* params)
{
}

void RenderObject::addElementToCanvas(const DrawParams* params, gdi::GDIElement* element)
{
    if (auto canvas = params->canvas)
    {
        const bool renderer_z_enabled = canvas->is_hardware_z_enabled();
        if (renderer_z_enabled)
        {
            const int32_t ui_z = params->ui_z;
            int32_t z_min, z_max;
            canvas->get_zrange(&z_min, &z_max);
            const int32_t canvas_z = std::clamp(ui_z, z_min, z_max);
            element->set_z(canvas_z);
        }
        canvas->add_element(element);
    }
}

LiteSpan<DiagnosticableTreeNode* const> RenderObject::get_diagnostics_children() const
{
    const eastl::vector<RenderObject*>& children_ = children;
    return { (DiagnosticableTreeNode* const*)children_.data(), children_.size() };
}

} // namespace skr::gui