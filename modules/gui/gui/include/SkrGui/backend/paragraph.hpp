#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
struct SKR_GUI_API IParagraph SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(IParagraph, "467730e3-29dd-4db1-8a81-4eb7c51ede64")
    virtual ~IParagraph() = default;
};
} // namespace skr::gui