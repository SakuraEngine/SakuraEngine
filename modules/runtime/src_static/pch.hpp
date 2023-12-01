#pragma once

#ifdef _WIN32
#include <intrin.h> // IWYU pragma: export
#include <new.h> // IWYU pragma: export
#endif

#include <chrono> // IWYU pragma: export
#include "SkrBase/misc/debug.h"  // IWYU pragma: export
#include "SkrRT/platform/guid.hpp" // IWYU pragma: export
#include <SkrRT/platform/filesystem.hpp> // IWYU pragma: export

#include "SkrRT/misc/log.h" // IWYU pragma: export
#include "SkrRT/misc/log.hpp" // IWYU pragma: export

#include "SkrRT/resource/resource_handle.h" // IWYU pragma: export
#include "SkrRT/serde/binary/writer.h" // IWYU pragma: export
#include "SkrRT/serde/binary/blob.h" // IWYU pragma: export
#include "SkrRT/serde/json/reader.h" // IWYU pragma: export
#include "SkrRT/serde/json/writer.h" // IWYU pragma: export

#include "SkrRT/misc/bits.hpp" // IWYU pragma: export
#include "SkrBase/math/rtm/scalarf.h" // IWYU pragma: export
#include "SkrBase/math/rtm/scalard.h" // IWYU pragma: export

#include <SkrRT/containers/concurrent_queue.hpp> // IWYU pragma: export
#include <SkrRT/containers/sptr.hpp> // IWYU pragma: export
#include <SkrRT/containers/string.hpp> // IWYU pragma: export
#include <SkrRT/containers/vector.hpp> // IWYU pragma: export
#include <SkrRT/containers/hashmap.hpp> // IWYU pragma: export

#include "SkrProfile/profile.h" // IWYU pragma: export