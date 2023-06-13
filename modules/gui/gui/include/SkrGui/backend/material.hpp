#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
struct SKR_GUI_API IMaterial SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(IMaterial, "51445807-4b0b-4a9e-a0d7-ee09589f2b3b")
    virtual ~IMaterial() = default;
};
} // namespace skr::gui