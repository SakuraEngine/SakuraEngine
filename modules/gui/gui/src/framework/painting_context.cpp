#include "SkrGui/framework/painting_context.hpp"
#include "SkrGui/framework/layer/geometry_layer.hpp"
#include "SkrGui/framework/layer/container_layer.hpp"

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
    SKR_UNIMPLEMENTED_FUNCTION()
}
void PaintingContext::add_layer(NotNull<Layer*> layer) SKR_NOEXCEPT
{
    _stop_recording();
    layer->unmount();
    _container_layer->add_child(layer);
}
void PaintingContext::push_layer(NotNull<ContainerLayer*> layer, ChildPaintingCallback callback, Offsetf offset) SKR_NOEXCEPT
{
    if (layer->has_children())
    {
        layer->remove_all_children();
    }
    _stop_recording();
    _container_layer->add_child(layer);

    PaintingContext ctx(layer);
    callback(ctx, offset);
    ctx._stop_recording();
}

// help functions
bool PaintingContext::_is_recording() const SKR_NOEXCEPT
{
    return _current_layer != nullptr;
}
void PaintingContext::_start_recording() SKR_NOEXCEPT
{
    _current_layer = SkrNew<GeometryLayer>();
    _container_layer->add_child(make_not_null(_current_layer));
}
void PaintingContext::_stop_recording() SKR_NOEXCEPT
{
    if (_is_recording())
    {
        _current_layer = nullptr;
    }
}
} // namespace skr::gui