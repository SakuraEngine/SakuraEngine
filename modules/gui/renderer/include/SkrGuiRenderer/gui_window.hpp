#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/interface/window.hpp"

struct SWindow;
namespace skr {
namespace gui {

struct SPlatformWindowDescriptor
{
    uint32_t posx = 0;
    uint32_t posy = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    const char8_t* title = nullptr;
    bool centered = false;
};

struct SKR_GUI_RENDERER_API SPlatformWindow : public IPlatformWindow
{
    virtual ~SPlatformWindow() SKR_NOEXCEPT = default;

    [[nodiscard]] static SPlatformWindow* Create(const SPlatformWindowDescriptor* desc) SKR_NOEXCEPT;
    [[nodiscard]] static SPlatformWindow* Import(SWindow* window) SKR_NOEXCEPT;
    static void Free(IPlatformWindow* window) SKR_NOEXCEPT;

    void get_extent(uint32_t* width, uint32_t* height) const SKR_NOEXCEPT final;
    void* get_native_handle() const SKR_NOEXCEPT final;

    // helpers
    bool initialize(const SPlatformWindowDescriptor* desc) SKR_NOEXCEPT;
    bool finalize();

    SWindow* handle = nullptr;
    bool imported = false;
};

} }