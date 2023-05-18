#include "SkrGuiRenderer/gui_window.hpp"
#include "platform/window.h"
#include "platform/memory.h"
#include "misc/log.h"

namespace skr {
namespace gui {

bool SPlatformWindow::initialize(const SPlatformWindowDescriptor* desc) SKR_NOEXCEPT
{
    SWindowDescroptor sdesc = {};
    sdesc.flags = SKR_WINDOW_RESIZABLE;
    sdesc.flags |= desc->centered ? SKR_WINDOW_CENTERED : 0;
    sdesc.width = desc->width;
    sdesc.height = desc->height;
    sdesc.posx = desc->posx;
    sdesc.posy = desc->posy;
    handle = skr_create_window(desc->title, &sdesc);
    return handle;
}

bool SPlatformWindow::finalize()
{
    if(handle) skr_free_window(handle);
    return true;
}

void SPlatformWindow::get_extent(uint32_t* out_width, uint32_t* out_height) const SKR_NOEXCEPT
{
    int32_t width = 0, height = 0;
    skr_window_get_extent(handle, &width, &height);
    if (out_width) *out_width = width;
    if (out_height) *out_height = height;
}

void* SPlatformWindow::get_native_handle() const SKR_NOEXCEPT
{
    return handle;
}

SPlatformWindow* SPlatformWindow::Create(const SPlatformWindowDescriptor* desc) SKR_NOEXCEPT
{
    auto window = SkrNew<SPlatformWindow>();
    if (window->initialize(desc))
    {
        return window;
    }
    else
    {
        SKR_LOG_ERROR("Failed to create GUI platform window!");
        return nullptr;
    }
}

SPlatformWindow* SPlatformWindow::Import(SWindow* handle_) SKR_NOEXCEPT
{
    auto window = SkrNew<SPlatformWindow>();
    if (window)
    {
        window->handle = handle_;
        window->imported = true;
        return window;
    }
    return nullptr;
}

void SPlatformWindow::Free(IPlatformWindow* window) SKR_NOEXCEPT
{
    if (auto window_impl = static_cast<SPlatformWindow*>(window))
    {
        if (!window_impl->imported) 
        {
            window_impl->finalize();
        }
        SkrDelete(window_impl);
    }
}

} }