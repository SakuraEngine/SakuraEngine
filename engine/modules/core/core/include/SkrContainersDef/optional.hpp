#pragma once
#include "SkrBase/containers/misc/optional.hpp"

namespace skr
{
using Nullopt [[sfinal_alias]] = ::skr::container::Nullopt;
template <typename T>
using Optional [[sfinal_alias]] = ::skr::container::Optional<T>;
}; // namespace skr