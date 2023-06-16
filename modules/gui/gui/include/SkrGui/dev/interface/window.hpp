#pragma once
#include "SkrGui/module.configure.h"
#include "platform/configure.h"

namespace skr::gui
{

struct SKR_GUI_API IPlatformWindow {
    virtual ~IPlatformWindow() SKR_NOEXCEPT;
    virtual void  get_extent(uint32_t* out_width, uint32_t* out_height) const SKR_NOEXCEPT = 0;
    virtual void* get_native_handle() const SKR_NOEXCEPT = 0;
};

} // namespace skr::gui
