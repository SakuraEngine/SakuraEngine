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
SKR_GUI_API IHitTestTarget : virtual public ::skr::rttr::IObject {
    SKR_RTTR_GENERATE_BODY()
    virtual void handle_event(NotNull<PointerEvent*> event, NotNull<HitTestEntry*> entry) = 0;
};

struct HitTestEntry {
    // 通常我们会让 hit_test 为 const，但是 event dispatch 为非 const，这里是为了方便代码书写
    inline HitTestEntry(const IHitTestTarget* target)
        : target(const_cast<IHitTestTarget*>(target))
    {
    }
    inline HitTestEntry(const IHitTestTarget* target, Matrix4 transform)
        : target(const_cast<IHitTestTarget*>(target))
        , transform(transform)
    {
    }

    IHitTestTarget*   target    = nullptr;
    Optional<Matrix4> transform = {};
};

struct HitTestResult {
    // add
    inline void add(HitTestEntry entry)
    {
        entry.transform = _get_last_transform();
        _path.add(entry);
    }

    // add helpers
    // bool HitTestFunc(HitTestResult* result, Offsetf local_position)
    template <typename HitTestFunc>
    inline bool add_with_raw_transform(Optional<Matrix4> matrix, Offsetf position, HitTestFunc&& hit_test)
    {
        bool is_hit;
        if (matrix)
        {
            push_transform(matrix.get());
            is_hit = hit_test(this, matrix.get().transform(position));
            pop_transform();
        }
        else
        {
            is_hit = hit_test(this, position);
        }
        return is_hit;
    }
    // bool HitTestFunc(HitTestResult* result, Offsetf local_position)
    template <typename HitTestFunc>
    inline bool add_with_paint_transform(Optional<Matrix4> matrix, Offsetf position, HitTestFunc&& hit_test)
    {
        if (matrix)
        {
            Matrix4 inv_matrix;
            if (matrix.get().try_inverse(inv_matrix))
            {
                return add_with_raw_transform(inv_matrix, position, hit_test);
            }
            else
            {
                SKR_LOG_ERROR(u8"Objects are not visible on screen and cannot be hit-tested");
            }
        }
        else
        {
            return add_with_raw_transform({}, position, hit_test);
        }
        return false;
    }
    // bool HitTestFunc(HitTestResult* result, Offsetf local_position)
    template <typename HitTestFunc>
    inline bool add_with_paint_offset(Optional<Offsetf> paint_offset, Offsetf position, HitTestFunc&& hit_test)
    {
        bool is_hit;
        if (paint_offset)
        {
            push_offset(-paint_offset.get());
            is_hit = hit_test(this, position - paint_offset);
            pop_transform();
        }
        else
        {
            is_hit = hit_test(this, position);
        }
        return is_hit;
    }

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
    inline Matrix4 _get_last_transform()
    {
        // globalize transforms
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

        return _transforms.empty() ? Matrix4::Identity() : _transforms.last();
    }

private:
    Array<HitTestEntry> _path;
    Array<Matrix4>      _transforms;       // global->local
    Array<Matrix4>      _local_transforms; // global->local
};

} // namespace gui sreflect
} // namespace skr sreflect