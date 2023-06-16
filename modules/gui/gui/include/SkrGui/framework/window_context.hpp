#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/dev/gdi/gdi_types.hpp"

namespace skr::gui
{
using skr::gdi::IGDIDevice;
using skr::gdi::IGDIRenderer;
using skr::gdi::ViewportRenderParams;

struct IPlatformWindow;
struct RenderWindow;

struct SKR_GUI_API WindowContext {
    struct Descriptor {
        IPlatformWindow* platform_window = nullptr;
        IGDIDevice*      gdi_device = nullptr;
        RenderWindow*    root_window = nullptr;
    };
    struct DrawParams {
        void* usr_data = nullptr;
    };
    struct RenderParams {

        const ViewportRenderParams* gdi_params = nullptr;
        void*                       usr_data = nullptr;
    };

    virtual ~WindowContext() SKR_NOEXCEPT = default;

    [[nodiscard]] static WindowContext* Create(const Descriptor* desc) SKR_NOEXCEPT;
    static void                         Free(WindowContext* context) SKR_NOEXCEPT;

    virtual IPlatformWindow* get_platform_window() const SKR_NOEXCEPT = 0;

    virtual void          set_root_element(struct RenderWindow* root) SKR_NOEXCEPT = 0;
    virtual RenderWindow* get_root_element() const SKR_NOEXCEPT = 0;

    virtual void draw(const DrawParams* params) SKR_NOEXCEPT = 0;
    virtual void render(IGDIRenderer* renderer, const RenderParams* params) SKR_NOEXCEPT = 0;
};

} // namespace skr::gui
