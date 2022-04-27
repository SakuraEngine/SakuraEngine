#pragma once

namespace dual
{
    template<class F>
    struct CallbackHelper;

    template<class R, class T, class... As>
    struct CallbackHelper<R(T::*)(As...) const>
    {
        template<R(T::* F)(As...) const>
        static R Call(void* u, As... as)
        {
            return (((const T*)u)->*F)(as...);
        }
    };

    template<class R, class T, class... As>
    struct CallbackHelper<R(T::*)(As...)>
    {
        template<R(T::* F)(As...)>
        static R Call(void* u, As... as)
        {
            return (((T*)u)->*F)(as...);
        }
    };
}

#define DUAL_LAMBDA(f) &dual::CallbackHelper<decltype(&decltype(f)::operator())>::Call<&decltype(f)::operator()>, &f