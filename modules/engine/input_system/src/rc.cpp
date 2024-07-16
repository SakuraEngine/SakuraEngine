#include "SkrInputSystem/input_value.hpp"

namespace skr {
namespace input {

RC::~RC() SKR_NOEXCEPT
{

}

uint32_t RC::add_refcount() SKR_NOEXCEPT
{
    return 1 + skr_atomic_fetch_add_relaxed(&rc, 1);
}

uint32_t RC::release() SKR_NOEXCEPT
{
    skr_atomic_fetch_add_relaxed(&rc, -1);
    return skr_atomic_load_acquire(&rc);
}

uint32_t RC::use_count() const SKR_NOEXCEPT
{
    return skr_atomic_load_acquire(&rc);
}

} }