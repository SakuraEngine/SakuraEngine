#pragma once
#include "interface/window.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, SGDIDevice, skr_gdi_device)

namespace skr {
namespace gui {
struct SWindowContextDescriptor
{
    IPlatformWindow* platform_window = nullptr;
    skr_gdi_device_id gdi_device = nullptr;
};

struct SKR_GUI_API SWindowContext
{
    virtual ~SWindowContext() SKR_NOEXCEPT = default;

    [[nodiscard]] static SWindowContext* Create(const SWindowContextDescriptor* desc) SKR_NOEXCEPT;
    static void Free(SWindowContext* context) SKR_NOEXCEPT;

    virtual IPlatformWindow* get_platform_window() const SKR_NOEXCEPT = 0;
    
    virtual void set_root_element(struct SRenderWindow* root) SKR_NOEXCEPT = 0;
    virtual SRenderWindow* get_root_element() const SKR_NOEXCEPT = 0;

    virtual void render() SKR_NOEXCEPT = 0;
};

} }

SKR_DECLARE_TYPE_ID(skr::gui::SWindowContext, skr_gui_window_context);
