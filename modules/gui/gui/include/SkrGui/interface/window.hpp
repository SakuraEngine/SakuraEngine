#pragma once
#include "SkrGui/module.configure.h"
#include "platform/configure.h"

namespace skr {
namespace gui {

struct SKR_GUI_API SPlatformWindow
{
    virtual ~SPlatformWindow() SKR_NOEXCEPT = default;

    virtual uint32_t get_width() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_height() const SKR_NOEXCEPT = 0;
    virtual void* get_native_handle() const SKR_NOEXCEPT = 0;
};

} }