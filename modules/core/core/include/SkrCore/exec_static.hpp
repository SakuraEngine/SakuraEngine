#pragma once
#include <type_traits>

namespace skr::__exec_static_helper
{
template <typename T>
struct ConstructStatic {
    inline ConstructStatic(const T& callable)
    {
        callable();
    }
};

template <typename T>
struct DestructStatic {
    inline DestructStatic(const T& callable)
        : _callable(callable)
    {
    }
    inline DestructStatic(T&& callable)
        : _callable(std::move(callable))
    {
    }

private:
    T _callable;
};
} // namespace skr::__exec_static_helper

#define SKR_EXEC_STATIC_COMBINE_IMPL(__X, __Y) __X##__Y
#define SKR_EXEC_STATIC_COMBINE(__X, __Y) SKR_EXEC_STATIC_COMBINE_IMPL(__X, __Y)
#define SKR_EXEC_STATIC_CTOR static ::skr::__exec_static_helper::ConstructStatic SKR_EXEC_STATIC_COMBINE(_exec_static_, __LINE__) = []()
#define SKR_EXEC_STATIC_DTOR static ::skr::__exec_static_helper::DestructStatic SKR_EXEC_STATIC_COMBINE(_exec_static_, __LINE__) = []()