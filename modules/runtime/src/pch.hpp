#pragma once
#include <intrin.h>
#include <chrono>

#include "platform/debug.h"
#ifdef _WIN32
#include "platform/windows/winheaders.h"
#endif

#include <platform/filesystem.hpp>

#include <containers/sptr.hpp>
#include <containers/string.hpp>
#include <containers/vector.hpp>
#include <containers/hashmap.hpp>

#include "tracy/Tracy.hpp"