#include "EASTL/optional.h"

namespace skr
{
    template<class T>
    using optional = eastl::optional<T>;
    constexpr auto nullopt = eastl::nullopt;
}