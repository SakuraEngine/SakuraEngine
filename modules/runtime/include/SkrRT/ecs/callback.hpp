#pragma once
#include "SkrBase/misc/traits.hpp"
#include "SkrRT/containers/function_ref.hpp"

namespace sugoi
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
        using raw = typename skr::FunctionTrait<T>::raw;
        return get_trampoline<T>(skr::type_t<raw>{});
    }
}

#define SUGOI_LAMBDA(f) sugoi::get_trampoline<decltype(f)>(), &f
#define SUGOI_LAMBDA_POINTER(f) sugoi::get_trampoline<decltype(*f)>(), f, nullptr, +[](void* u, EIndex entityCount) { SkrDelete(((decltype(f))u)); }