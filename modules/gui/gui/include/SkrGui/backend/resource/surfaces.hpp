#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
struct INativeDevice;
enum class EResourceState
{
    Requested,
    Loading,
    Initializing,
    Okay,
    Destroying,
};

struct SKR_GUI_API ISurface {
    virtual ~ISurface() = default;

    virtual EResourceState state() const SKR_NOEXCEPT = 0;
    virtual INativeDevice* device() const SKR_NOEXCEPT = 0;
};

} // namespace skr::gui