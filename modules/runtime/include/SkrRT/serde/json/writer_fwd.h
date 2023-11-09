#pragma once
#include "SkrRT/config.h"
#include <type_traits>

struct skr_json_writer_t;
typedef char8_t skr_json_writer_char_t;
typedef size_t skr_json_writer_size_t;

#if defined(__cplusplus)

namespace skr
{
namespace json
{
template <class T, class = void>
struct WriteTrait;
} // namespace json
} // namespace skr

#endif // defined(__cplusplus)