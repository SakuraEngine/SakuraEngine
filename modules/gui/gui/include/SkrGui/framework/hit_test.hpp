#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/matrix.hpp"

namespace skr sreflect
{
namespace gui sreflect
{
struct PointerEvent;
struct HitTestEntry;

// TODO. rttr
struct IHitTestTarget {
    void handle_event(NotNull<PointerEvent*> event, NotNull<HitTestEntry*> entry);
};

// TODO. keyboard & gamepad Input 怎么支持

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

} // namespace gui sreflect
} // namespace skr sreflect