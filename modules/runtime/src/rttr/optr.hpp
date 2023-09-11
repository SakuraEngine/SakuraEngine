#pragma once

namespace skr::rttr
{
template <typename T>
struct OPtr {

private:
    T*     _ptr           = nullptr;
    size_t _header_offset = 0;
};
} // namespace skr::rttr