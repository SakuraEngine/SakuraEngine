#pragma once
#include "SkrContainersDef/path.hpp" // IWYU pragma: export
#include "SkrContainers/string.hpp"  // IWYU pragma: export

// format
namespace skr::container
{
template <>
struct Formatter<skr::Path>
{
    template<typename TString>
    inline static void format(TString& out, const skr::Path& path, typename TString::ViewType spec)
    {
        out.append(path.string());
    }
};
}

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <>
struct Serialize<Path>
{
    inline static void read(ArchiveRead& r, Path& p)
    {
        StringView str;
        SKR_FAST_CHECK(r.str_view(str), );
        p = skr::Path(str);
    }

    inline static void write(ArchiveWrite& w, const Path& p)
    {
        w.str(p.string());
    }
};
} // namespace skr