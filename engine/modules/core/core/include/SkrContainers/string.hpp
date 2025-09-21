#pragma once
#include "SkrContainersDef/string.hpp" // IWYU pragma: export

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <>
struct Serialize<skr::String>
{
    inline static void read(ArchiveRead& r, skr::String& v)
    {
        StringView str;
        SKR_FAST_CHECK(r.str_view(str), );
        v = str;
    }
    inline static void write(ArchiveWrite& w, const skr::String& v)
    {
        w.str(v);
    }
};
} // namespace skr
