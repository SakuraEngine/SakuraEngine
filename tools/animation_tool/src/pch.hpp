#pragma once
#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>  // include API for expanding a gltf file path
#endif

#include "SkrMemory/memory.h" // IWYU pragma: export
#include "SkrBase/misc/debug.h"  // IWYU pragma: export
#include "SkrRT/platform/thread.h" // IWYU pragma: export
#include "SkrRT/platform/filesystem.hpp" // IWYU pragma: export
#include "SkrRT/misc/make_zeroed.hpp" // IWYU pragma: export
#include "SkrRT/misc/types.h" // IWYU pragma: export
#include "SkrRT/misc/log.hpp" // IWYU pragma: export
#include "SkrBase/misc/hash.h" // IWYU pragma: export
#include "SkrBase/misc/defer.hpp" // IWYU pragma: export

#include <SkrRT/platform/filesystem.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/sptr.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/vector.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/string.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/hashmap.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/btree.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/concurrent_queue.hpp> // IWYU pragma: export
#include "SkrRT/containers_new/span.hpp" // IWYU pragma: export
#include "SkrRT/containers_new/function_ref.hpp" // IWYU pragma: export

#include "SkrRT/io/ram_io.hpp" // IWYU pragma: export
#include "SkrRT/resource/resource_header.hpp" // IWYU pragma: export
#include "SkrRT/serde/binary/reader.h" // IWYU pragma: export
#include "SkrRT/serde/binary/writer.h" // IWYU pragma: export
#include "SkrRT/serde/json/reader.h" // IWYU pragma: export
#include "SkrRT/serde/json/writer.h" // IWYU pragma: export

#include "SkrProfile/profile.h" // IWYU pragma: export

/* TODO: MAYBE THIS IS AN XMAKE PCH BUG
#include "cgltf/cgltf.h" // IWYU pragma: export

#include "SkrToolCore/asset/cook_system.hpp" // IWYU pragma: export

#include "SkrAnim/ozz/skeleton.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/containers/map.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/containers/set.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/containers/vector.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/log.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/maths/math_ex.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/maths/simd_math.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/io/archive.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/io/stream.h" // IWYU pragma: export

#include "SkrAnimTool/ozz/raw_track.h" // IWYU pragma: export
#include "SkrAnimTool/ozz/tools/import2ozz.h" // IWYU pragma: export
#include "SkrAnimTool/ozz/track_builder.h" // IWYU pragma: export
#include "SkrAnimTool/ozz/track_optimizer.h" // IWYU pragma: export
#include "SkrAnim/ozz/track.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/maths/soa_transform.h" // IWYU pragma: export
#include "SkrAnim/ozz/base/memory/unique_ptr.h" // IWYU pragma: export
#include "SkrAnimTool/ozz/skeleton_builder.h" // IWYU pragma: export

#include "gltf/extern/json.hpp" // IWYU pragma: export
*/