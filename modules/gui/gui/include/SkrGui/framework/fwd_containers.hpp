#pragma once
#include <type_traits>
#include "SkrGui/module.configure.h"
#ifdef SKR_GUI_IMPL
#define CONTAINER_LITE_IMPL
#endif
#include "containers/lite.hpp"

namespace skr {
namespace gui {

using namespace skr::lite;

} }