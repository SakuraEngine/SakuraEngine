#pragma once
#include <chrono> // IWYU pragma: export

#include "SkrRT/platform/memory.h" // IWYU pragma: export
#include "SkrRT/platform/debug.h" // IWYU pragma: export
#include "SkrRT/io/ram_io.hpp" // IWYU pragma: export
#include "SkrRT/misc/make_zeroed.hpp" // IWYU pragma: export
#include "SkrRT/misc/types.h" // IWYU pragma: export
#include "SkrRT/misc/log.hpp" // IWYU pragma: export

#include <SkrRT/platform/filesystem.hpp> // IWYU pragma: export
#include <SkrRT/containers/sptr.hpp> // IWYU pragma: export
#include <SkrRT/containers/vector.hpp> // IWYU pragma: export
#include <SkrRT/containers/string.hpp> // IWYU pragma: export
#include <SkrRT/containers/hashmap.hpp> // IWYU pragma: export

#include "SkrRT/serde/json/reader.h" // IWYU pragma: export
#include "SkrRT/serde/json/writer.h" // IWYU pragma: export

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
    #endif
    #include <wtypes.h> // IWYU pragma: export
    #include <unknwn.h> // IWYU pragma: export
    #include <winbase.h> // IWYU pragma: export
    #include <winioctl.h> // IWYU pragma: export
#endif
#include "dxc/dxcapi.h" // IWYU pragma: export


#include "tracy/Tracy.hpp" // IWYU pragma: export