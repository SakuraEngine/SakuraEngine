#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "cgpu/api.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "platform/window.h"

namespace skr::gui
{
using skr::render_graph::RenderGraph;
struct SkrRenderWindow;

// 渲染设备，渲染的
struct SKR_GUI_RENDERER_API SkrRenderDevice final {
    // init & shutdown
    void init();
    void shutdown();

    // create view
    SkrRenderWindow* create_window(SWindowHandle window);
    void             destroy_window(SkrRenderWindow* view);

    // getter
    inline CGPUInstanceId cgpu_instance() const { return _cgpu_instance; }
    inline CGPUAdapterId  cgpu_adapter() const { return _cgpu_adapter; }
    inline CGPUDeviceId   cgpu_device() const { return _cgpu_device; }
    inline CGPUQueueId    cgpu_queue() const { return _cgpu_queue; }
    inline RenderGraph*   render_graph() const { return _render_graph; }

private:
    CGPUInstanceId _cgpu_instance = nullptr;
    CGPUAdapterId  _cgpu_adapter = nullptr;
    CGPUDeviceId   _cgpu_device = nullptr;
    CGPUQueueId    _cgpu_queue = nullptr;

    RenderGraph* _render_graph = nullptr;
};
} // namespace skr::gui