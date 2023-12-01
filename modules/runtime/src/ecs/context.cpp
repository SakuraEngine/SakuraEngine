#include "context.hpp"
#include "SkrRT/ecs/dual.h"

dual_context_t* g_dual_ctx;

SKR_RUNTIME_API dual_context_t* dual_get_context()
{
    static dual_context_t* ctx = dual_initialize();
    return ctx;
}

namespace dual
{
type_registry_t& type_registry_t::get()
{
    return dual_get_context()->typeRegistry;
}
pool_t& get_default_pool()
{
    return dual_get_context()->normalPool;
}
pool_t& get_default_pool_small()
{
    return dual_get_context()->smallPool;
}
pool_t& get_default_pool_large()
{
    return dual_get_context()->largePool;
}
scheduler_t& scheduler_t::get()
{
    return dual_get_context()->scheduler;
}
skr::String& get_error()
{
    return dual_get_context()->error;
}
} // namespace dual

dual_context_t::dual_context_t()
    : normalPool(dual::kFastBinSize, dual::kFastBinCapacity)
    , largePool(dual::kLargeBinSize, dual::kLargeBinCapacity)
    , smallPool(dual::kSmallBinSize, dual::kSmallBinCapacity)
    , typeRegistry(smallPool)
    , scheduler()
{
}

extern "C" {
dual_context_t* dual_initialize()
{
    if (g_dual_ctx)
        return g_dual_ctx;
    return g_dual_ctx = new dual_context_t();
}

void dual_shutdown()
{
    if (auto ctx = g_dual_ctx)
        delete ctx;
}
}