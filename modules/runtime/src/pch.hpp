#pragma once

#ifdef _WIN32
#include <intrin.h> // IWYU pragma: keep
#include <new.h> // IWYU pragma: keep
#include "platform/windows/winheaders.h" // IWYU pragma: keep
#endif

#include <chrono> // IWYU pragma: keep
#include "SkrRT/platform/debug.h" // IWYU pragma: keep
#include "SkrRT/platform/guid.hpp" // IWYU pragma: keep
#include <SkrRT/platform/filesystem.hpp> // IWYU pragma: keep

#include "SkrRT/misc/log.h" // IWYU pragma: keep
#include "SkrRT/misc/log.hpp" // IWYU pragma: keep

#include <SkrRT/containers/concurrent_queue.h> // IWYU pragma: keep
#include <SkrRT/containers/sptr.hpp> // IWYU pragma: keep
#include <SkrRT/containers/string.hpp> // IWYU pragma: keep
#include <SkrRT/containers/vector.hpp> // IWYU pragma: keep
#include <SkrRT/containers/hashmap.hpp> // IWYU pragma: keep

#include "tracy/Tracy.hpp" // IWYU pragma: keep