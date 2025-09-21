#pragma once
#include "SkrContainersDef/array.hpp"

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <typename T, size_t N>
struct Serialize<skr::Array<T, N>>
{
    using ArrayType = T[N];
    inline static void read(ArchiveRead& r, skr::Array<T, N>& v)
    {
        r.value<T[N]>(*reinterpret_cast<ArrayType*>(v.data()));
    }
    inline static void write(ArchiveWrite& w, const skr::Array<T, N>& v)
    {
        w.value<T[N]>(*reinterpret_cast<const ArrayType*>(v.data()));
    }
};
} // namespace skr