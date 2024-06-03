// TODO. ！！！！！！！！！！！ MOVE TO SKR_SERDE
#pragma once
#include "SkrBase/config.h"
#include <type_traits>

struct skr_arena_t;
typedef struct SJsonReader SJsonReader;

namespace skr
{
namespace json
{
template <class T, class = void>
struct ReadTrait;
} // namespace json
} // namespace skr

struct SJsonWriter;
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