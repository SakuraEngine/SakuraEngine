#include "SkrGui/framework/painting_context.hpp"
#include "SkrGui/framework/layer/geometry_layer.hpp"
#include "SkrGui/framework/layer/container_layer.hpp"
#include "SkrGui/framework/layer/offset_layer.hpp"
#include "SkrGui/framework/render_object/render_object.hpp"

namespace skr::gui
{
PaintingContext::PaintingContext(NotNull<ContainerLayer*> container_layer) SKR_NOEXCEPT
    : _container_layer(container_layer)
{
}
ICanvas* PaintingContext::canvas() SKR_NOEXCEPT
{
    if (!_is_recording())
    {
        _start_recording();
    }
    return _current_layer->canvas();
}
void PaintingContext::paint_child(NotNull<RenderObject*> child, Offsetf offset) noexcept
{
    if (child->is_repaint_boundary())
    {
        _stop_recording();
        _composite_child(child, offset);
    }
    else if (child->was_repaint_boundary())
    {
        child->set_layer(nullptr);
        _paint_with_context(child, offset);
    }
    else
    {
        _paint_with_context(child, offset);
    }
}
void PaintingContext::add_layer(NotNull<Layer*> layer) SKR_NOEXCEPT
{
    _stop_recording();
    _append_layer(layer);
}
void PaintingContext::push_layer(NotNull<ContainerLayer*> layer, ChildPaintingCallback callback, Offsetf offset) SKR_NOEXCEPT
{
    if (layer->has_children())
    {
        layer->remove_all_children();
    }
    _stop_recording();
    _append_layer(layer);

    PaintingContext ctx(layer);
    callback(ctx, offset);
    ctx._stop_recording();
}

// repaint layer or just update properties
void PaintingContext::repaint_composited_child(NotNull<RenderObject*> child)
{
    // update layer
    auto child_layer = child->layer()->type_cast_fast<OffsetLayer>();
    if (child_layer)
    {
        child_layer->remove_all_children();
        child_layer = child->update_layer(child_layer);
    }
    else
    {
        child_layer = child->update_layer(nullptr);
        child->set_layer(child_layer);
    }
    child->cancel_needs_layer_update();

    // paint
    PaintingContext ctx(child_layer);
    ctx._paint_with_context(child, Offsetf::Zero());
    ctx._stop_recording();
}
void PaintingContext::update_layer_properties(NotNull<RenderObject*> child)
{
    auto child_layer = child->layer()->type_cast_fast<OffsetLayer>();
    child_layer      = child->update_layer(child_layer);
    child->cancel_needs_layer_update();
}

// help functions
bool PaintingContext::_is_recording() const SKR_NOEXCEPT
{
    return _current_layer != nullptr;
}
void PaintingContext::_start_recording() SKR_NOEXCEPT
{
    _current_layer = SkrNew<GeometryLayer>();
    _container_layer->add_child(_current_layer);
}
void PaintingContext::_stop_recording() SKR_NOEXCEPT
{
    if (_is_recording())
    {
        _current_layer = nullptr;
    }
}
void PaintingContext::_composite_child(NotNull<RenderObject*> child, Offsetf offset) SKR_NOEXCEPT
{
    if (child->needs_paint() || !child->was_repaint_boundary())
    {
        repaint_composited_child(child);
    }
    else if (child->needs_layer_update())
    {
        update_layer_properties(child);
    }

    auto child_offset_layer = child->layer()->type_cast_fast<OffsetLayer>();
    child_offset_layer->set_offset(offset);
    _append_layer(child_offset_layer);
}
void PaintingContext::_append_layer(NotNull<Layer*> layer)
{
    layer->unmount();
    _container_layer->add_child(layer);
}
void PaintingContext::_paint_with_context(NotNull<RenderObject*> render_object, Offsetf offset)
{
    render_object->cancel_needs_paint();
    render_object->_was_repaint_boundary = render_object->is_repaint_boundary();
    render_object->paint(*this, offset);
}
} // namespace skr::gui