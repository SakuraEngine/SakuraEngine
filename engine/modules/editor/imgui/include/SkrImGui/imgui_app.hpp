#pragma once
#include "SkrImGui/imgui_system_event_handler.hpp"
#include "SkrCore/dirty.hpp"
#include "SkrRenderer/render_app.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <chrono>

namespace skr
{
struct ImGuiRendererBackend;
struct SystemApp;
struct SystemWindow;

struct ImGuiRendererBackendRGTextureData
{
    CGPUTextureId texture = nullptr;
    CGPUTextureViewId srv = nullptr;
    bool first_update = true;
};

struct SKR_IMGUI_API ImGuiApp : public skr::RenderApp
{
public:
    ImGuiApp(const SystemWindowCreateInfo& main_wnd_create_info, SRenderDeviceId render_device, skr::RG::RenderGraphBuilder& builder);
    ~ImGuiApp();

    virtual bool initialize(const char* backend = nullptr) override;
    virtual void shutdown() override;

    void pump_message();
    void render_imgui();

public:
    // imgui functional
    void apply_context();
    void enable_nav(bool enable = true);
    void enable_docking(bool enable = true);
    void enable_multi_viewport(bool enable = true);
    void enable_ini_file(bool enable = true);
    void enable_log_file(bool enable = true);
    void enable_transparent_docking(bool enable = true);
    void enable_always_tab_bar(bool enable = true);
    void enable_move_window_by_blank_area(bool enable = true);
    void enable_high_dpi(bool enable = true);
    void set_load_action(ECGPULoadAction action);

    // getter
    inline bool is_created() const { return _context != nullptr; }
    inline const Trigger& want_exit() const { return _event_handler ? _event_handler->want_exit() : _want_exit; }
    inline ImGuiContext* context() const { return _context; }
    inline SystemWindow* main_window() const { return _main_window; }

    mutable skr::String _clipboard;

private:
    // render
    void create_pipeline();
    void add_render_pass(
        ImGuiViewport* vp,
        RG::RenderGraph* render_graph,
        CGPURootSignatureId root_sig,
        CGPURenderPipelineId render_pipeline);
    void create_texture(ImTextureData* tex_data);
    void destroy_texture(ImTextureData* tex_data);
    void update_texture(ImTextureData* tex_data);
    ECGPULoadAction _load_action = CGPU_LOAD_ACTION_CLEAR;
    CGPURootSignatureId _root_signature = nullptr;
    CGPURenderPipelineId _render_pipeline = nullptr;
    CGPUSamplerId _static_sampler = nullptr;

    SystemWindowCreateInfo _main_window_info;
    uint32_t _main_window_index = 0;
    SystemWindow* _main_window = nullptr;

    // context
    ImGuiContext* _context = nullptr;
    // system integration
    ImGuiSystemEventHandler* _event_handler = nullptr;

    // dirty & trigger (for legacy mode)
    Trigger _pixel_size_changed = {};
    Trigger _want_exit = {};

    // Helper methods
    void UpdateMouseCursor();
    void SetupIMECallbacks();
    void UpdateIMEState();
    bool _ime_active_state = false;

    // Time tracking
    std::chrono::steady_clock::time_point last_frame_time_;
};
} // namespace skr