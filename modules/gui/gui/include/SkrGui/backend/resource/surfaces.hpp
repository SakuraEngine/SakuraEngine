#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"

// type
namespace skr::gui
{
enum class EResourceState
{
    Requested,
    Loading,
    Initializing,
    Okay,
    Destroying,
};
}

// interface
namespace skr::gui
{
struct SKR_GUI_API ISurface {
    virtual ~ISurface() = default;
};

struct SKR_GUI_API IImage : public ISurface {
};

struct SKR_GUI_API ITexture : public ISurface {
};

struct SKR_GUI_API IMaterial : public ISurface {
};

} // namespace skr::gui