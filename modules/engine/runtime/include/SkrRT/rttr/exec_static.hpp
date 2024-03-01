#pragma once

namespace skr::rttr
{
template <typename T>
struct ExecStatic {
    inline ExecStatic(const T& t)
    {
        t();
    }
};
} // namespace skr::rttr

#define SKR_RTTR_MARCO_COMBINE(__X, __Y) __X##__Y
#define SKR_RTTR_EXEC_STATIC static ::skr::rttr::ExecStatic SKR_RTTR_MARCO_COMBINE(__register_type_, __LINE__) = []()