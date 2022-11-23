#pragma once
#include <type_traits>
#include "platform/configure.h"

struct skr_binary_reader_t;

namespace skr
{
namespace binary
{
template <class T>
std::enable_if_t<!std::is_enum_v<T>, int> ReadValue(skr_binary_reader_t* reader, T& value)
{
    static_assert(!sizeof(T), "ReadValue not implemented for this type");
    return -1;
}
}
}