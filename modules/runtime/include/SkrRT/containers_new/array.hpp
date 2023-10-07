#pragma once
#include "SkrRT/containers_new/skr_allocator.hpp"
#include "SkrBase/containers/array/array.hpp"

namespace skr
{
template <typename T>
using Array = container::Array<T, SkrAllocator>;
}

// serde
#include "SkrRT/serde/binary/serde.h"
#include "SkrRT/serde/binary/reader_fwd.h"
#include "SkrRT/serde/binary/writer_fwd.h"

// binary reader
namespace skr::binary
{
} // namespace skr