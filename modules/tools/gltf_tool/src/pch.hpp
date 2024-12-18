#pragma once
#include "SkrGraphics/api.h"       // IWYU pragma: export
#include "cgltf/cgltf.h"           // IWYU pragma: export
#include "MeshOpt/meshoptimizer.h" // IWYU pragma: export

#include "SkrCore/platform/vfs.h"     // IWYU pragma: export
#include "SkrOS/filesystem.hpp"     // IWYU pragma: export
#include "SkrBase/misc/defer.hpp"   // IWYU pragma: export
#include "SkrCore/log.hpp"          // IWYU pragma: export
#include "SkrTask/parallel_for.hpp" // IWYU pragma: export
#include "SkrTask/fib_task.hpp"     // IWYU pragma: export
#include "SkrRT/io/ram_io.hpp"      // IWYU pragma: export

#include "SkrCore/module/module.hpp"        // IWYU pragma: export
#include "SkrRT/resource/config_resource.h" // IWYU pragma: export

#include "SkrContainers/string.hpp"  // IWYU pragma: export
#include "SkrContainers/vector.hpp"  // IWYU pragma: export
#include "SkrContainers/hashmap.hpp" // IWYU pragma: export

#include "SkrProfile/profile.h" // IWYU pragma: export