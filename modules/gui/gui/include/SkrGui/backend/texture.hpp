#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
struct SKR_GUI_API ITexture SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(ITexture, "cb3face9-d1ac-408c-8b2f-a3a06b5fb999")
    virtual ~ITexture() = default;
    virtual Size size() = 0;
};
} // namespace skr::gui