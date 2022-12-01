#pragma once
#include <type_traits>

namespace skr
{
    template<class T>
    struct type_t{ using type = T; };

    template<class T, class = void>
    struct is_complete : std::false_type {};

    template<class T>
    struct is_complete<T, std::enable_if_t<sizeof(T)>> : std::true_type {};

    template<class T>
    constexpr bool is_complete_v = is_complete<T>::value;
};