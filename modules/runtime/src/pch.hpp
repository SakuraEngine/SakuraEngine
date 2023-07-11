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

#include <SkrRT/containers/concurrent_queue.h>
#include <SkrRT/containers/sptr.hpp>
#include <SkrRT/containers/string.hpp>
#include <SkrRT/containers/vector.hpp>
#include <SkrRT/containers/hashmap.hpp>

#include "tracy/Tracy.hpp"