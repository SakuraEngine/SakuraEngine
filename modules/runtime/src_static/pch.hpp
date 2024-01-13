#pragma once

#ifdef _WIN32
#include <intrin.h> // IWYU pragma: export
#include <new.h> // IWYU pragma: export
#endif

#include <chrono> // IWYU pragma: export
#include "SkrBase/misc/debug.h"  // IWYU pragma: export
#include "SkrCore/guid.hpp" // IWYU pragma: export
#include <SkrOS/filesystem.hpp> // IWYU pragma: export

#include "SkrCore/log.h" // IWYU pragma: export
#include "SkrCore/log.hpp" // IWYU pragma: export

#include "SkrRT/resource/resource_handle.h" // IWYU pragma: export
#include "SkrRT/serde/binary/writer.h" // IWYU pragma: export
#include "SkrRT/serde/binary/blob.h" // IWYU pragma: export
#include "SkrRT/serde/json/reader.h" // IWYU pragma: export
#include "SkrRT/serde/json/writer.h" // IWYU pragma: export

#include "SkrRT/misc/bits.hpp" // IWYU pragma: export
#include "SkrBase/math/rtm/scalarf.h" // IWYU pragma: export
#include "SkrBase/math/rtm/scalard.h" // IWYU pragma: export

#include <SkrContainers/concurrent_queue.hpp> // IWYU pragma: export
#include <SkrContainers/sptr.hpp> // IWYU pragma: export
#include <SkrContainers/string.hpp> // IWYU pragma: export
#include <SkrContainers/vector.hpp> // IWYU pragma: export
#include <SkrContainers/hashmap.hpp> // IWYU pragma: export

#include "SkrProfile/profile.h" // IWYU pragma: export