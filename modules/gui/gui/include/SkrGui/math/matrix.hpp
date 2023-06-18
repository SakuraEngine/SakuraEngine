#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
// row major matrix
struct SKR_ALIGNAS(16) Matrix4 {
    float _m[4][4];
};
} // namespace skr::gui