#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API PipelineOwner final {

    // schedule
    void schedule_layout_for(NotNull<RenderObject*> node) SKR_NOEXCEPT;
    void schedule_paint_for(NotNull<RenderObject*> node) SKR_NOEXCEPT;

    // flush
    void flush_layout();
    void flush_paint();

private:
    Array<RenderObject*> _nodes_needing_layout;
    Array<RenderObject*> _nodes_needing_paint;
};
} // namespace skr::gui