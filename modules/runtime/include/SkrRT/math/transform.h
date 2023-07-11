#pragma once
#include "SkrRT/math/vector.h"
#include "SkrRT/math/quat.h"
#include "SkrRT/math/rtm/qvvf.h"

namespace skr
{
namespace math
{
    inline rtm::qvvf load(skr_transform_t t)
    {
        return rtm::qvv_set(load(t.rotation), load(t.translation), load(t.scale));
    }

    inline void store(rtm::qvvf v, skr_transform_t& result)
    {
        store(v.rotation, result.rotation);
        store(v.translation, result.translation);
        store(v.scale, result.scale);
    }
}
}