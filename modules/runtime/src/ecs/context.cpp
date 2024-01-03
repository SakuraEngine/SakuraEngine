#include "context.hpp"
#include "SkrRT/ecs/sugoi.h"

sugoi_context_t* g_sugoi_ctx;

SKR_RUNTIME_API sugoi_context_t* sugoi_get_context()
{
    static sugoi_context_t* ctx = sugoi_initialize();
    return ctx;
}

namespace sugoi
{
type_registry_t& type_registry_t::get()
{
    return sugoi_get_context()->typeRegistry;
}
pool_t& get_default_pool()
{
    return sugoi_get_context()->normalPool;
}
pool_t& get_default_pool_small()
{
    return sugoi_get_context()->smallPool;
}
pool_t& get_default_pool_large()
{
    return sugoi_get_context()->largePool;
}
scheduler_t& scheduler_t::get()
{
    return sugoi_get_context()->scheduler;
}
skr::String& get_error()
{
    return sugoi_get_context()->error;
}
} // namespace sugoi

sugoi_context_t::sugoi_context_t()
    : normalPool(sugoi::kFastBinSize, sugoi::kFastBinCapacity)
    , largePool(sugoi::kLargeBinSize, sugoi::kLargeBinCapacity)
    , smallPool(sugoi::kSmallBinSize, sugoi::kSmallBinCapacity)
    , typeRegistry(smallPool)
    , scheduler()
{
}

extern "C" {
sugoi_context_t* sugoi_initialize()
{
    if (g_sugoi_ctx)
        return g_sugoi_ctx;
    return g_sugoi_ctx = new sugoi_context_t();
}

void sugoi_shutdown()
{
    if (auto ctx = g_sugoi_ctx)
        delete ctx;
}
}