#include "SkrRTTR/iobject.hpp"
#include "SkrBase/atomic/atomic.h"

namespace skr::rttr
{
uint32_t Object::embedded_rc_add_ref()
{
    return 1 + skr_atomic_fetch_add_relaxed(&_object_ref_count, 1);
}
uint32_t Object::embedded_rc_release_ref()
{
    skr_atomic_fetch_add_relaxed(&_object_ref_count, -1);
    return skr_atomic_load_acquire(&_object_ref_count);
}
uint32_t Object::embedded_rc_ref_count() const
{
    return skr_atomic_load_acquire(&_object_ref_count);
}
} // namespace skr::rttr