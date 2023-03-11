#pragma once
#include "SkrGui/module.configure.h"
#include "platform/configure.h"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIDevice, skr_gdi_device)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, IGDIRenderer, skr_gdi_renderer)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, IPlatformWindow, skr_gui_platform_window);
SKR_DECLARE_TYPE_ID_FWD(skr::gui, RenderWindow, skr_gui_render_window);
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIViewport, skr_gdi_viewport)

typedef struct skr_gui_window_context_descriptor_t {
    skr_gui_platform_window_id platform_window = nullptr;
    skr_gdi_device_id gdi_device = nullptr;
    skr_gui_render_window_id root_window = nullptr;
} skr_gui_window_context_descriptor_t;

typedef struct skr_gui_window_context_draw_params_t {
    void* usr_data SKR_IF_CPP(= nullptr);
} skr_gui_window_context_draw_params_t;

typedef struct skr_gui_window_context_render_params_t {
    const struct skr_gdi_viewport_render_params_t* gdi_params SKR_IF_CPP(= nullptr);
    void* usr_data SKR_IF_CPP(= nullptr);
} skr_gui_window_context_render_params_t;

namespace skr {
namespace gui {

struct SKR_GUI_API WindowContext
{
    using Descriptor = skr_gui_window_context_descriptor_t;
    using DrawParams = skr_gui_window_context_draw_params_t;
    using RenderParams = skr_gui_window_context_render_params_t;

    virtual ~WindowContext() SKR_NOEXCEPT = default;

    [[nodiscard]] static WindowContext* Create(const Descriptor* desc) SKR_NOEXCEPT;
    static void Free(WindowContext* context) SKR_NOEXCEPT;

    virtual skr_gui_platform_window_id get_platform_window() const SKR_NOEXCEPT = 0;
    
    virtual void set_root_element(struct RenderWindow* root) SKR_NOEXCEPT = 0;
    virtual RenderWindow* get_root_element() const SKR_NOEXCEPT = 0;

    virtual void draw(const DrawParams* params) SKR_NOEXCEPT = 0;
    virtual void render(skr_gdi_renderer_id renderer, const RenderParams* params) SKR_NOEXCEPT = 0;
};

} }

SKR_DECLARE_TYPE_ID(skr::gui::WindowContext, skr_gui_window_context);