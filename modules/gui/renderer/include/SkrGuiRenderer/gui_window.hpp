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
    const char* title = nullptr;
    bool centered = false;
};

struct SKR_GUI_RENDERER_API SPlatformWindow : public IPlatformWindow
{
    virtual ~SPlatformWindow() SKR_NOEXCEPT = default;

    [[nodiscard]] SPlatformWindow* Create(const SPlatformWindowDescriptor* desc) SKR_NOEXCEPT;
    [[nodiscard]] SPlatformWindow* Import(SWindow* window) SKR_NOEXCEPT;
    void Free(SPlatformWindow* window) SKR_NOEXCEPT;
};

} }