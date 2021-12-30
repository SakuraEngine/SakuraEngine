#pragma once
#include <EASTL/tuple.h>

namespace swa
{
namespace wa_edge
{
template <typename Func>
class utilx;
template <typename Ret, typename... Args>
class utilx<Ret(Args...)>
{
public:
    constexpr static size_t n_args = sizeof...(Args);
    FORCEINLINE static void fill_linkage(SWAHostFunctionDescriptor& linkage,
        const char* module_name,
        const char* function_name,
        Ret (*function)(Args...))
    {
        linkage.function_name = function_name;
        linkage.module_name = module_name;
        linkage.proc = reinterpret_cast<void*>(function);
        // linkage.backend_wrappers.wa_edge = &detail::wrap_helper<Ret(Args...)>::wrap_fn;
        // linkage.signatures.wa_edge = detail::m3_signature<Ret, Args...>::value;
    }
};
} // namespace wa_edge
} // namespace swa