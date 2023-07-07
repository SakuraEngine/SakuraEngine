#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct INativeDevice;

struct SKR_GUI_API PipelineOwner final {
    PipelineOwner(INativeDevice* native_device) SKR_NOEXCEPT;

    // schedule
    void schedule_layout_for(NotNull<RenderObject*> node) SKR_NOEXCEPT;
    void schedule_paint_for(NotNull<RenderObject*> node) SKR_NOEXCEPT;

    // flush
    void flush_layout();
    void flush_paint();

    inline INativeDevice* native_device() const SKR_NOEXCEPT { return _native_device; }

private:
    Array<RenderObject*> _nodes_needing_layout = {};
    Array<RenderObject*> _nodes_needing_paint  = {};

    INativeDevice* _native_device = nullptr;
};
} // namespace skr::gui