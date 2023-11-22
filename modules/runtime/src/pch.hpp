#pragma once

#ifdef _WIN32
#include <intrin.h> // IWYU pragma: export
#include <new.h> // IWYU pragma: export
#include "platform/windows/winheaders.h" // IWYU pragma: export
#endif

#include <random> // IWYU pragma: export
#include <chrono> // IWYU pragma: export
#include "SkrBase/misc/debug.h"  // IWYU pragma: export
#include "SkrRT/platform/guid.hpp" // IWYU pragma: export
#include <SkrRT/platform/filesystem.hpp> // IWYU pragma: export

#include "SkrRT/ecs/entity.hpp" // IWYU pragma: export
#include "SkrRT/ecs/dual.h" // IWYU pragma: export

#include "SkrRT/misc/log.h" // IWYU pragma: export
#include "SkrRT/misc/log.hpp" // IWYU pragma: export
#include "SkrRT/serde/json/reader.h" // IWYU pragma: export

#include <SkrRT/containers/concurrent_queue.h> // IWYU pragma: export
#include <SkrRT/containers/sptr.hpp> // IWYU pragma: export
#include <SkrRT/containers/string.hpp> // IWYU pragma: export
#include <SkrRT/containers/vector.hpp> // IWYU pragma: export
#include <SkrRT/containers/hashmap.hpp> // IWYU pragma: export
#include <SkrRT/containers/btree.hpp> // IWYU pragma: export

#include <EASTL/functional.h> // IWYU pragma: export
#include <EASTL/bitset.h> // IWYU pragma: export
#include <EASTL/string.h> // IWYU pragma: export
#include <EASTL/vector.h> // IWYU pragma: export
#include <EASTL/fixed_vector.h> // IWYU pragma: export

#include "cgpu/cgpu_config.h" // IWYU pragma: export
#ifdef CGPU_USE_D3D12
    #include <d3d12.h> // IWYU pragma: export
    #include <dxgi.h>  // IWYU pragma: export
#endif

#include "SkrProfile/profile.h" // IWYU pragma: export