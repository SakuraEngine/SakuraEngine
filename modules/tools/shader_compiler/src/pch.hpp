#pragma once
#include <chrono>            // IWYU pragma: export
#include "SkrGraphics/api.h" // IWYU pragma: export

#include "SkrCore/memory/memory.h"                // IWYU pragma: export
#include "SkrBase/misc/debug.h"              // IWYU pragma: export
#include <SkrOS/filesystem.hpp>              // IWYU pragma: export
#include "SkrBase/misc/make_zeroed.hpp"      // IWYU pragma: export
#include "SkrCore/log.hpp"                   // IWYU pragma: export
#include "SkrTask/parallel_for.hpp"          // IWYU pragma: export
#include "SkrRT/resource/resource_factory.h" // IWYU pragma: export
#include "SkrRT/io/ram_io.hpp"               // IWYU pragma: export

#include <SkrContainers/sptr.hpp>    // IWYU pragma: export
#include <SkrContainers/vector.hpp>  // IWYU pragma: export
#include <SkrContainers/string.hpp>  // IWYU pragma: export
#include <SkrContainers/hashmap.hpp> // IWYU pragma: export

#include "SkrSerde/json/reader.h" // IWYU pragma: export
#include "SkrSerde/json/writer.h" // IWYU pragma: export

#include <SkrContainers/stl_vector.hpp> // IWYU pragma: export
#include <SkrContainers/stl_string.hpp> // IWYU pragma: export

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <wtypes.h>   // IWYU pragma: export
    #include <unknwn.h>   // IWYU pragma: export
    #include <winbase.h>  // IWYU pragma: export
    #include <winioctl.h> // IWYU pragma: export
#endif
#include "dxc/dxcapi.h" // IWYU pragma: export

#include "SkrProfile/profile.h" // IWYU pragma: export