#pragma once
#include <type_traits>
#include "platform/configure.h"

struct skr_binary_reader_t;

namespace skr
{
namespace binary
{
template <class T, class = void>
struct ReadHelper;
}
}