// TODO. ！！！！！！！！！！！ MOVE TO SKR_SERDE
#pragma once
#include "SkrBase/config.h"
#include <type_traits>

struct skr_arena_t;

namespace skr
{
namespace json
{
struct Reader;
struct Writer;

template <class T, class = void>
struct ReadTrait;
} // namespace json
} // namespace skr

typedef char8_t SJsonCharType;
typedef size_t  SJsonSizeType;

namespace skr
{
namespace json
{
template <class T, class = void>
struct WriteTrait;
} // namespace json
} // namespace skr