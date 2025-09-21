#pragma once
#include "SkrContainersDef/map.hpp"
#include "SkrContainersDef/sparse_vector.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderer/render_device.h"
#include "SkrSystem/system_app.h"

namespace skr
{
struct SKR_RENDERER_API RenderApp : public skr::SystemApp
{
public:
    using RenderGraph = skr::render_graph::RenderGraph;
    RenderApp(SRenderDeviceId render_device, skr::render_graph::RenderGraphBuilder& builder);
    ~RenderApp();

    virtual bool initialize(const char* backend = nullptr) override;
    virtual void shutdown() override;

    uint32_t open_window(const SystemWindowCreateInfo& create_info);
    void close_window(uint32_t idx);
    void close_window(skr::SystemWindow* window);
    void close_all_windows();
    
    skr::SystemWindow* get_window(uint32_t idx);
    inline skr::SystemWindow* get_main_window() { return get_window(0); }

    uint32_t acquire_frame(skr::SystemWindow* window);
    void acquire_frames();

    void present(skr::SystemWindow* window);
    void present_all();

    uint32_t backbuffer_index(skr::SystemWindow* window);
    CGPUSwapChainId get_swapchain(skr::SystemWindow* window);
    CGPUTextureId get_backbuffer(skr::SystemWindow* window);
    inline auto render_graph() { return _graph; }

protected:
    struct SwapchainManager : public skr::ISystemEventHandler
    {
    public:
        SwapchainManager(RenderApp& app) SKR_NOEXCEPT;
        void handle_event(const SkrSystemEvent& event) SKR_NOEXCEPT override;
        CGPUSwapChainId get_swapchain(skr::SystemWindow* window);
        uint32_t acquire_frame(skr::SystemWindow* window);
        uint32_t backbuffer_index(skr::SystemWindow* window);
        void present(skr::SystemWindow* window);
        void destory_swapchain(skr::SystemWindow* window);
        void recreate_swapchain(skr::SystemWindow* window);
        struct SwapChain
        {
            CGPUSurfaceId _surface = nullptr;
            CGPUSwapChainId _swapchain = nullptr;
            CGPUSemaphoreId _semaphore = nullptr;
            float2 _size = float2(0, 0);
            uint32_t _current_index = 0;
        };
    private:
        friend struct RenderApp;
        skr::Map<skr::SystemWindow*, SwapChain> _swapchains;
        RenderApp& _app;
    } swapchain_manager;
    ECGPUFormat _backbuffer_format = CGPU_FORMAT_R8G8B8A8_UNORM;
    skr::SparseVector<skr::SystemWindow*> _windows;
    SRenderDeviceId _render_device;
    skr::render_graph::RenderGraphBuilder _graph_builder;
    RenderGraph* _graph = nullptr;
};
} // namespace skr