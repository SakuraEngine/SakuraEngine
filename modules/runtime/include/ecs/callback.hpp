#pragma once
#include "utils/traits.hpp"
#include "utils/function_ref.hpp"

namespace dual
{
    template<class F, class R, class... Args>
    auto get_trampoline(skr::type_t<R(Args...)>)
    {
        return +[](void* u, Args... args) -> R {
            return std::invoke(*reinterpret_cast<F*>(u), std::forward<Args>(args)...);
        };
    }

    template<class F>
    auto get_trampoline()
    {
        using T = std::decay_t<F>;
        using raw = typename skr::function_trait<T>::raw;
        return get_trampoline<T>(skr::type_t<raw>{});
    }
}

#define DUAL_LAMBDA(f) dual::get_trampoline<decltype(f)>(), &f
#define DUAL_LAMBDA_POINTER(f) dual::get_trampoline<decltype(*f)>(), f, nullptr, +[](void* u, EIndex entityCount) { SkrDelete(((decltype(f))u)); }