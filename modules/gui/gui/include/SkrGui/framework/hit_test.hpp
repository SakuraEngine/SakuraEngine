#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/matrix.hpp"

namespace skr
{
namespace gui
{

struct HitTestEntry {
    RenderObject*     target    = nullptr;
    Optional<Matrix4> transform = {};
};

struct HitTestResult {

private:
    Array<HitTestEntry> _path;
    Array<Matrix4>      _transforms;
    Array<Matrix4>      _local_transforms;
};

} // namespace gui
} // namespace skr