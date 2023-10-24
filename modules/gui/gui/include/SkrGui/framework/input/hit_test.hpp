#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/matrix.hpp"
#include "SkrGui/math/geometry.hpp"
#ifndef __meta__
    #include "SkrGui/framework/input/hit_test.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct PointerEvent;
struct HitTestEntry;

sreflect_struct(
    "guid": "72127868-ac63-46e0-a8e1-cb5c7c2c4382"
)
IHitTestTarget : virtual public ::skr::rttr::IObject {
    SKR_RTTR_GENERATE_BODY()
    void handle_event(NotNull<PointerEvent*> event, NotNull<HitTestEntry*> entry);
};

struct HitTestEntry {
    IHitTestTarget*   target    = nullptr;
    Optional<Matrix4> transform = {};
};

struct HitTestResult {
    // add ops

    // transform stack
    inline void push_transform(const Matrix4& matrix)
    {
        _local_transforms.stack_push(matrix);
    }
    inline void push_offset(Offsetf offset)
    {
        _local_transforms.stack_push(Matrix4::Translate(offset.x, offset.y, 0.f));
    }
    inline void pop_transform()
    {
        if (!_local_transforms.empty())
        {
            _local_transforms.stack_pop();
        }
        else
        {
            _transforms.stack_pop();
        }
    }

private:
    // tools
    inline void _globalize_transforms()
    {
        if (!_local_transforms.empty())
        {
            Matrix4 last = _transforms.last();
            for (const auto matrix : _local_transforms)
            {
                last = matrix * last;
                _transforms.add(last);
            }
            _local_transforms.clear();
        }
    }

private:
    Array<HitTestEntry> _path;
    Array<Matrix4>      _transforms;       // global->local
    Array<Matrix4>      _local_transforms; // global->local
};

} // namespace gui sreflect
} // namespace skr sreflect