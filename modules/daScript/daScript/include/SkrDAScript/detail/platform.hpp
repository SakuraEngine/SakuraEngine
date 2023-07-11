#pragma once
#include "SkrDAScript/module.configure.h"
#include "SkrRT/platform/configure.h"
#include <type_traits>
#include <utility>
#include "misc/types.h"

#ifndef VECMATH_FINLINE
#define VECMATH_FINLINE FORCEINLINE
#endif
#include <vecmath/dag_vecMath.h>

namespace skr {
namespace das {

using reg4f = ::vec4f;
using reg4i = ::vec4i;

} // namespace das
} // namespace skr
