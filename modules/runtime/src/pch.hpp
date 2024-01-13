#pragma once

#ifdef _WIN32
#include <intrin.h> // IWYU pragma: export
#include <new.h> // IWYU pragma: export
#include "platform/windows/winheaders.h" // IWYU pragma: export
#endif

#include <random> // IWYU pragma: export
#include <chrono> // IWYU pragma: export
#include "SkrBase/misc/debug.h"  // IWYU pragma: export
#include "SkrCore/guid.hpp" // IWYU pragma: export
#include <SkrOS/filesystem.hpp> // IWYU pragma: export

#include "SkrRT/ecs/entity.hpp" // IWYU pragma: export
#include "SkrRT/ecs/sugoi.h" // IWYU pragma: export

#include "SkrCore/log.h" // IWYU pragma: export
#include "SkrCore/log.hpp" // IWYU pragma: export
#include "SkrRT/serde/json/reader.h" // IWYU pragma: export

#include <SkrContainers/concurrent_queue.hpp> // IWYU pragma: export
#include <SkrContainers/sptr.hpp> // IWYU pragma: export
#include <SkrContainers/string.hpp> // IWYU pragma: export
#include <SkrContainers/vector.hpp> // IWYU pragma: export
#include <SkrContainers/hashmap.hpp> // IWYU pragma: export
#include <SkrContainers/btree.hpp> // IWYU pragma: export

#include <SkrContainers/stl_vector.hpp> // IWYU pragma: export
#include <SkrContainers/stl_string.hpp> // IWYU pragma: export
#include <SkrContainers/stl_deque.hpp> // IWYU pragma: export
#include <SkrContainers/stl_list.hpp> // IWYU pragma: export

#include "SkrGraphics/cgpu_config.h" // IWYU pragma: export
#ifdef CGPU_USE_D3D12
    #include <d3d12.h> // IWYU pragma: export
    #include <dxgi.h>  // IWYU pragma: export
#endif

#include "SkrProfile/profile.h" // IWYU pragma: export