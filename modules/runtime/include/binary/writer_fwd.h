#pragma once
#include <type_traits>
#include "platform/configure.h"

struct skr_binary_writer_t;

namespace skr::binary
{
template <class T, class = void>
struct WriteHelper;
}