#pragma once
#include <EASTL/span.h>

namespace skr
{
using eastl::span;
}

#include "EASTL/vector.h"
#include "binary/blob_fwd.h"

namespace skr::binary
{
template <class T>
struct BlobBuilderType<skr::span<T>>
{
    using type = eastl::vector<typename BlobBuilderType<T>::type>;
};
}