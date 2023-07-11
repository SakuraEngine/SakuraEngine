#pragma once

#ifdef _WIN32
#include <intrin.h>
#include <new.h>
#include "SkrRT/platform/windows/winheaders.h"
#endif

#include <chrono>
#include "SkrRT/platform/debug.h"
#include "SkrRT/platform/guid.hpp"
#include <SkrRT/platform/filesystem.hpp>

#include "SkrRT/misc/log.h"
#include "SkrRT/misc/log.hpp"

#include <containers/concurrent_queue.h>
#include <containers/sptr.hpp>
#include <containers/string.hpp>
#include <containers/vector.hpp>
#include <containers/hashmap.hpp>

#include "tracy/Tracy.hpp"