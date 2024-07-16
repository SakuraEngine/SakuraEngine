#pragma once
#include "SkrBase/config.h"
#include "SkrRT/config.h"
#include <type_traits>
#include <utility>

#ifndef VECMATH_FINLINE
    #define VECMATH_FINLINE FORCEINLINE
#endif
#include <vecmath/dag_vecMath.h>

namespace skr
{
namespace das
{

using reg4f = ::vec4f;
using reg4i = ::vec4i;

} // namespace das
} // namespace skr
