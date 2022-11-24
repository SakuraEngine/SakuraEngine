#pragma once
#include <type_traits>
#include "platform/configure.h"

struct skr_binary_writer_t;

namespace skr::binary
{
template <class T>
using TParamType = std::conditional_t<std::is_fundamental_v<T> || std::is_enum_v<T>, T, const T&>;

template <class T, class = void>
struct WriteHelper;
}