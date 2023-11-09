#pragma once
#include "SkrRT/config.h"
#include "OpenString/text.h"
#include "OpenString/format.h"
#include "SkrRT/misc/types.h"
#include "SkrBase/misc/hash.hpp"

namespace skr
{
using namespace ostr;
using string      = ostr::text;
using string_view = ostr::text_view;

template <>
struct Hash<string> {
    inline size_t operator()(const string& x) const
    {
        return ostr::hash_sequence_crc64(x.c_str(), x.size());
    }
};

template <>
struct Hash<string_view> {
    inline size_t operator()(const string_view& x) const
    {
        return ostr::hash_sequence_crc64(x.raw().data(), x.size());
    }
};

} // namespace skr