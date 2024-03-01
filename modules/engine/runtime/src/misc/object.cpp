#include "SkrRT/misc/types.h"
#include "SkrOS/atomic.h"
namespace skr {
uint32_t SObjectHeader::add_refcount()
{
    auto last = skr_atomicu32_add_relaxed(&rc, 1);
    return last + 1;
}
uint32_t SObjectHeader::release()
{
    skr_atomicu32_add_relaxed(&rc, -1);
    return skr_atomicu32_load_acquire(&rc);
}
}