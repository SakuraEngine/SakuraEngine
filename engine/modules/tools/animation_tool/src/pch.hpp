#pragma once
#ifdef _WIN32

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <windows.h> // include API for expanding a gltf file path
#endif

#include "SkrCore/memory/memory.h"      // IWYU pragma: export
#include "SkrBase/misc/debug.h"         // IWYU pragma: export
#include "SkrOS/thread.h"               // IWYU pragma: export
#include "SkrOS/filesystem.hpp"         // IWYU pragma: export
#include "SkrBase/misc/make_zeroed.hpp" // IWYU pragma: export
#include "SkrCore/log.hpp"              // IWYU pragma: export
#include "SkrBase/misc/hash.h"          // IWYU pragma: export
#include "SkrBase/misc/defer.hpp"       // IWYU pragma: export

#include <SkrOS/filesystem.hpp>               // IWYU pragma: export
#include <SkrContainers/vector.hpp>           // IWYU pragma: export
#include <SkrContainers/string.hpp>           // IWYU pragma: export
#include <SkrContainers/hashmap.hpp>          // IWYU pragma: export
#include <SkrContainers/btree.hpp>            // IWYU pragma: export
#include <SkrContainers/concurrent_queue.hpp> // IWYU pragma: export
#include "SkrContainers/span.hpp"             // IWYU pragma: export
#include "SkrContainers/function_ref.hpp"     // IWYU pragma: export

#include "SkrRuntime/io/ram_io.hpp"                // IWYU pragma: export
#include "SkrRuntime/resource/resource_header.hpp" // IWYU pragma: export

#include "SkrProfile/profile.h" // IWYU pragma: export

#include "cgltf/cgltf.h" // IWYU pragma: export

#include "SkrToolCore/cook_system/cook_system.hpp" // IWYU pragma: export

#include "SkrAnim/ozz/skeleton.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/containers/map.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/containers/set.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/containers/vector.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/log.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/maths/math_ex.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/maths/simd_math.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/io/archive.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/io/stream.h" // IWYU pragma: export

#include "SkrAnim/ozz/track.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/maths/soa_transform.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/memory/unique_ptr.h" // IWYU pragma: export
#include "SkrAnimTool/ozz/skeleton_builder.h" // IWYU pragma: export
