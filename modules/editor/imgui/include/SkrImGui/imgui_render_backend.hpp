#pragma once
#include <SkrGraphics/cgpux.h>
#include <SkrGraphics/api.h>
#include <SkrRenderGraph/frontend/render_graph.hpp>
#include <SkrContainers/string.hpp>
#include <SkrContainers/optional.hpp>
#include <SkrBase/math.h>
#include <SkrCore/memory/rc.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace skr
{
struct ImGuiWindowBackend;

struct SKR_IMGUI_API ImGuiRendererBackend {
    SKR_RC_IMPL();

    // ctor & dtor
    ImGuiRendererBackend()          = default;
    virtual ~ImGuiRendererBackend() = default;

    // setup io
    virtual void setup_io(ImGuiIO& io) = 0;

    // rendering utils
    virtual void wait_rendering_done()                 = 0;
    virtual void acquire_next_frame(ImGuiViewport* vp) = 0;
    virtual void begin_frame()                         = 0;
    virtual void end_frame()                           = 0;

    // main window api
    virtual void create_main_window(ImGuiViewport* wnd)              = 0;
    virtual void destroy_main_window(ImGuiViewport* wnd)             = 0;
    virtual void resize_main_window(ImGuiViewport* wnd, ImVec2 size) = 0;
    virtual void render_main_window(ImGuiViewport* wnd)              = 0;

    // multi viewport api
    virtual void create_window(ImGuiViewport* viewport)              = 0;
    virtual void destroy_window(ImGuiViewport* viewport)             = 0;
    virtual void resize_window(ImGuiViewport* viewport, ImVec2 size) = 0;
    virtual void render_window(ImGuiViewport* viewport, void*)       = 0;

    // texture api
    virtual uint32_t backbuffer_count() const                 = 0;
    virtual void     create_texture(ImTextureData* tex_data)  = 0;
    virtual void     destroy_texture(ImTextureData* tex_data) = 0;
    virtual void     update_texture(ImTextureData* tex_data)  = 0;
};

struct ImGuiRendererBackendRGConfig {
    skr::render_graph::RenderGraph* render_graph     = nullptr;
    CGPUQueueId                     queue            = nullptr;
    ECGPUFormat                     format           = CGPU_FORMAT_R8G8B8A8_UNORM;
    uint32_t                        backbuffer_count = 2;

    Optional<CGPUSamplerId>             static_sampler = {}; // default: linear
    Optional<CGPUShaderEntryDescriptor> vs             = {}; // default: shaders/imgui_vertex
    Optional<CGPUShaderEntryDescriptor> ps             = {}; // default: shaders/imgui_fragment
};

struct SKR_IMGUI_API ImGuiRendererBackendRG : ImGuiRendererBackend {
    // ctor & dtor
    ImGuiRendererBackendRG();
    ~ImGuiRendererBackendRG();

    // init & shutdown
    bool has_init() const;
    void init(const ImGuiRendererBackendRGConfig& config);
    void shutdown();

    // real present
    void present_all();
    void present_main_viewport();
    void present_sub_viewports();

    // viewport info
    CGPUTextureId   get_backbuffer(ImGuiViewport* vp);
    uint32_t        get_backbuffer_index(ImGuiViewport* vp);
    CGPUSwapChainId get_swapchain(ImGuiViewport* vp);
    void            set_load_action(ImGuiViewport* vp, ECGPULoadAction load_action);

    //==> ImGuiRendererBackend API
    // setup io
    void setup_io(ImGuiIO& io) override;

    // rendering utils
    void wait_rendering_done() override;
    void acquire_next_frame(ImGuiViewport* vp) override;
    void begin_frame() override;
    void end_frame() override;

    // main window api
    void create_main_window(ImGuiViewport* vp) override;
    void destroy_main_window(ImGuiViewport* vp) override;
    void resize_main_window(ImGuiViewport* vp, ImVec2 size) override;
    void render_main_window(ImGuiViewport* vp) override;

    // multi viewport api
    void create_window(ImGuiViewport* vp) override;
    void destroy_window(ImGuiViewport* vp) override;
    void resize_window(ImGuiViewport* vp, ImVec2 size) override;
    void render_window(ImGuiViewport* vp, void*) override;

    // texture api
    uint32_t backbuffer_count() const override;
    void     create_texture(ImTextureData* tex_data) override;
    void     destroy_texture(ImTextureData* tex_data) override;
    void     update_texture(ImTextureData* tex_data) override;
    //==> ImGuiRendererBackend API

private:
    // config
    CGPUQueueId                     _gfx_queue         = nullptr;
    skr::render_graph::RenderGraph* _render_graph      = nullptr;
    uint32_t                        _backbuffer_count  = 2;
    ECGPUFormat                     _backbuffer_format = CGPU_FORMAT_R8G8B8A8_UNORM;

    // state
    CGPURootSignatureId  _root_sig        = nullptr;
    CGPURenderPipelineId _render_pipeline = nullptr;
    CGPUXBindTableId     _bind_table      = nullptr;
    CGPUSamplerId        _static_sampler  = nullptr;
};

} // namespace skr