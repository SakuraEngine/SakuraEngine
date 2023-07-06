#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct ICanvasService;
struct IResourceService;

struct SKR_GUI_API PipelineOwner final {
    PipelineOwner(ICanvasService* canvas_service) SKR_NOEXCEPT;

    // schedule
    void schedule_layout_for(NotNull<RenderObject*> node) SKR_NOEXCEPT;
    void schedule_paint_for(NotNull<RenderObject*> node) SKR_NOEXCEPT;

    // flush
    void flush_layout();
    void flush_paint();

    inline ICanvasService* canvas_service() const SKR_NOEXCEPT { return _canvas_service; }

private:
    Array<RenderObject*> _nodes_needing_layout;
    Array<RenderObject*> _nodes_needing_paint;

    ICanvasService* _canvas_service;
};
} // namespace skr::gui