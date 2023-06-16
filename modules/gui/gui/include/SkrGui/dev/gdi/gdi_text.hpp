#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gdi
{
struct IGDIRenderer;

struct SKR_GUI_API GDIText {
    static bool     Initialize(IGDIRenderer* renderer);
    static bool     Finalize();
    static GDIText* Get();
};
} // namespace skr::gdi
