#include "SkrRenderer/render_app.hpp"

namespace skr
{

RenderApp::RenderApp(SRenderDeviceId render_device, skr::RG::RenderGraphBuilder& builder)
    : _render_device(render_device), swapchain_manager(*this), _graph_builder(builder)
{

}

RenderApp::~RenderApp()
{
}

uint32_t RenderApp::open_window(const SystemWindowCreateInfo& create_info)
{
    auto new_window = window_manager->create_window(create_info);
    swapchain_manager.recreate_swapchain(new_window);
    auto index = _windows.add(new_window).index();
    return index;
}

void RenderApp::close_window(uint32_t idx)
{
    if (auto to_close = get_window(idx))
    {
        close_window(to_close);
    }
}

void RenderApp::close_window(skr::SystemWindow* to_close)
{
    _windows.remove(to_close);
    swapchain_manager.destory_swapchain(to_close);
    window_manager->destroy_window(to_close);
}

void RenderApp::close_all_windows()
{
    for (auto to_close : _windows)
    {
        swapchain_manager.destory_swapchain(to_close);
        window_manager->destroy_window(to_close);
    }
    _windows.clear();
}

skr::SystemWindow* RenderApp::get_window(uint32_t idx)
{
    return _windows.at(idx);
}

uint32_t RenderApp::acquire_frame(skr::SystemWindow* window)
{
    return swapchain_manager.acquire_frame(window);
}

void RenderApp::acquire_frames()
{
    for (auto window : _windows)
    {
        acquire_frame(window);
    }
}

uint32_t RenderApp::backbuffer_index(skr::SystemWindow* window)
{
    return swapchain_manager.backbuffer_index(window);
}

void RenderApp::present(skr::SystemWindow* window)
{
    return swapchain_manager.present(window);
}

void RenderApp::present_all()
{
    for (auto window : _windows)
    {
        swapchain_manager.present(window);
    }
}

CGPUSwapChainId RenderApp::get_swapchain(skr::SystemWindow* window)
{
    return swapchain_manager.get_swapchain(window);
}

CGPUTextureId RenderApp::get_backbuffer(skr::SystemWindow* window)
{
    auto swapchain = swapchain_manager.get_swapchain(window);
    const auto index = swapchain_manager.backbuffer_index(window);
    return swapchain->back_buffers[index];
}

bool RenderApp::initialize(const char* backend)
{
    if (!SystemApp::initialize(backend))
        return false;
    get_event_queue()->add_handler(&swapchain_manager);
    _graph = RG::RenderGraph::create(
    [=, this](skr::RG::RenderGraphBuilder& bd) {
        bd = _graph_builder;
    });
    return true;
}

void RenderApp::shutdown()
{
    get_event_queue()->remove_handler(&swapchain_manager);
    skr::RG::RenderGraph::destroy(_graph);
    SystemApp::shutdown();
}

RenderApp::SwapchainManager::SwapchainManager(RenderApp& app) SKR_NOEXCEPT
    : _app(app)
{
}

void RenderApp::SwapchainManager::handle_event(const SkrSystemEvent& event) SKR_NOEXCEPT
{
    if (event.window.type == SKR_SYSTEM_EVENT_WINDOW_RESIZED ||
        event.window.type == SKR_SYSTEM_EVENT_WINDOW_RESTORED ||
        event.window.type == SKR_SYSTEM_EVENT_WINDOW_MAXIMIZED)
    {
        auto wm = _app.get_window_manager();
        recreate_swapchain(wm->get_window_by_native_handle(event.window.window_native_handle));
    }
    else if (event.window.type == SKR_SYSTEM_EVENT_WINDOW_CLOSE_REQUESTED)
    {
        // auto wm = _app.get_window_manager();
        // auto window = wm->get_window_by_native_handle(event.window.window_native_handle);
        // _app.close_window(window);
    }
}

CGPUSwapChainId RenderApp::SwapchainManager::get_swapchain(skr::SystemWindow* window)
{
    auto found = _swapchains.find(window);
    return found ? found.value()._swapchain : nullptr;
}

uint32_t RenderApp::SwapchainManager::acquire_frame(skr::SystemWindow* window)
{
    if (auto exist = _swapchains.find(window))
    {
        CGPUAcquireNextDescriptor desc = {
            .signal_semaphore = exist.value()._semaphore,
            .fence = nullptr
        };
        auto window_index = exist.index();
        auto backbuffer_index = cgpu_acquire_next_image(exist.value()._swapchain, &desc);
        exist.ref().value._current_index = backbuffer_index;
        {
            auto swapchain = exist.value()._swapchain;
            auto backbuffer = swapchain->back_buffers[backbuffer_index];
            auto imported = _app._graph->create_texture(
                [=](RG::RenderGraph& g, RG::TextureBuilder& builder) {
                    builder.set_name(u8"backbuffer")
                        .import(backbuffer, CGPU_RESOURCE_STATE_UNDEFINED)
                        .allow_render_target();
                });
            _app._graph->add_before_execute_callback([backbuffer_index, swapchain, window_index, imported](RenderGraph& graph) {
                graph.add_present_pass(
                    [=](RG::RenderGraph& g, RG::PresentPassBuilder& builder) {
                        skr::String pass_name = skr::format(u8"present-{}", window_index);
                        builder.set_name((const char8_t*)pass_name.c_str())
                            .swapchain(swapchain, backbuffer_index)
                            .texture(imported, true);
                    });
            });
        }
        return backbuffer_index;
    }
    return UINT32_MAX;
}

uint32_t RenderApp::SwapchainManager::backbuffer_index(skr::SystemWindow* window)
{
    if (auto exist = _swapchains.find(window))
    {
        return exist.value()._current_index;
    }
    return UINT32_MAX;
}

void RenderApp::SwapchainManager::present(skr::SystemWindow* window)
{
    if (auto exist = _swapchains.find(window))
    {
        const auto backbuffer_index = exist.value()._current_index;
        auto swapchain = exist.value()._swapchain;
        CGPUQueuePresentDescriptor present = {
            .swapchain = swapchain,
            .index = (uint8_t)backbuffer_index
        };
        cgpu_queue_present(_app._render_device->get_gfx_queue(), &present);
    }
}

void RenderApp::SwapchainManager::destory_swapchain(skr::SystemWindow* window)
{
    auto cgpu_device = _app._render_device->get_cgpu_device();
    auto gfx_queue = _app._render_device->get_gfx_queue();
    if (auto s = _swapchains.find(window))
    {
        cgpu_wait_queue_idle(gfx_queue);
        cgpu_free_swapchain(s.value()._swapchain);
        cgpu_free_surface(cgpu_device, s.value()._surface);
        cgpu_free_semaphore(s.value()._semaphore);
        _swapchains.remove_at(s.index());
    }
}

void RenderApp::SwapchainManager::recreate_swapchain(skr::SystemWindow* window)
{
    destory_swapchain(window);

    auto cgpu_device = _app._render_device->get_cgpu_device();
    auto gfx_queue = _app._render_device->get_gfx_queue();
    const auto phys_size = window->get_physical_size();
    auto surface = cgpu_surface_from_native_view(_app._render_device->get_cgpu_device(), window->get_native_view());
    CGPUSwapChainDescriptor chain_desc = {};
    chain_desc.surface = surface;
    chain_desc.present_queues = &gfx_queue;
    chain_desc.present_queues_count = 1;
    chain_desc.width = phys_size.x;
    chain_desc.height = phys_size.y;
    chain_desc.image_count = 2;
    chain_desc.enable_vsync = false;
    chain_desc.format = _app._backbuffer_format;
    auto swapchain = cgpu_create_swapchain(cgpu_device, &chain_desc);
    _swapchains.add(window, SwapChain{ ._surface = surface, ._swapchain = swapchain, ._semaphore = cgpu_create_semaphore(cgpu_device), ._size = phys_size, ._current_index = 0 });
}

} // namespace skr