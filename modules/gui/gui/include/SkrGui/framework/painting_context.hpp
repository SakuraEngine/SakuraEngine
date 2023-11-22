#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
struct SKR_GUI_API PaintingContext final {
    using ChildPaintingCallback = FunctionRef<void(const PaintingContext&, Offsetf)>;

    PaintingContext(NotNull<ContainerLayer*> container_layer) SKR_NOEXCEPT;

    ICanvas* canvas() SKR_NOEXCEPT;
    void     paint_child(NotNull<RenderObject*> child, Offsetf offset) SKR_NOEXCEPT;
    void     add_layer(NotNull<Layer*> layer) SKR_NOEXCEPT;
    void     push_layer(NotNull<ContainerLayer*> layer, ChildPaintingCallback callback, Offsetf offset) SKR_NOEXCEPT;

    // repaint layer or just update properties
    static void repaint_composited_child(NotNull<RenderObject*> child);
    static void update_layer_properties(NotNull<RenderObject*> child);

private:
    // help functions
    bool _is_recording() const SKR_NOEXCEPT;
    void _start_recording() SKR_NOEXCEPT;
    void _stop_recording() SKR_NOEXCEPT;
    void _composite_child(NotNull<RenderObject*> child, Offsetf offset) SKR_NOEXCEPT;
    void _append_layer(NotNull<Layer*> layer);
    void _paint_with_context(NotNull<RenderObject*> render_object, Offsetf offset);

private:
    ContainerLayer* _container_layer = nullptr;
    GeometryLayer*  _current_layer   = nullptr;
};
} // namespace skr::gui