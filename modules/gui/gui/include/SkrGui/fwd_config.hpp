#pragma once
//********************************************************
//**                  外部细节隔离头                     **
//********************************************************

// std includes
#include <type_traits>
#include <cinttypes>
#include <cstddef>
#include <limits>

// export macro
#include "SkrGui/module.configure.h"

// skr types
#include "misc/types.h"

// skr containers
#ifdef SKR_GUI_IMPL
    #define CONTAINER_LITE_IMPL
#endif
#include "containers/lite.hpp"

namespace skr::gui
{
// Lite container
using skr::lite::HashMapStorage;
using skr::lite::LiteOptional;
using skr::lite::LiteSpan;
using skr::lite::TextStorage;
using skr::lite::VectorStorage;

// TODO. use Size class
using BoxSizeType = skr_float2_t;

} // namespace skr::gui