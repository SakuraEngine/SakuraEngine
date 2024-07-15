#pragma once
#include "SkrArchive/binary/common.h"

#if defined(__cplusplus)
    #include "SkrContainersDef/vector.hpp"

namespace skr::archive
{

using BinaryWriteError  = BinaryErrorCode;
using BinaryWriteResult = BinaryResult;

struct SKR_STATIC_API _BinaryWriter {
    using CharType = char8_t;
    using SizeType = size_t;
};

} // namespace skr::archive
#endif