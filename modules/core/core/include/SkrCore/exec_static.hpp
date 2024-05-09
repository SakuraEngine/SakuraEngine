#pragma once

namespace skr::__exec_static_helper
{
template <typename T>
struct ExecStatic {
    inline ExecStatic(const T& t)
    {
        t();
    }
};
} // namespace skr::__exec_static_helper

#define SKR_EXEC_STATIC_COMBINE(__X, __Y) __X##__Y
#define SKR_EXEC_STATIC static ::skr::__exec_static_helper::ExecStatic SKR_EXEC_STATIC_COMBINE(_exec_static_, __LINE__) = []()