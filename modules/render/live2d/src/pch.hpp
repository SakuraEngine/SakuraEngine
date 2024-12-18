#pragma once

#ifdef _WIN32
#include <intrin.h> // IWYU pragma: export
#include <new.h> // IWYU pragma: export
#endif

#include "SkrBase/misc/debug.h"  // IWYU pragma: export
#include <SkrOS/filesystem.hpp> // IWYU pragma: export
#include "SkrCore/memory/memory.h" // IWYU pragma: export
#include "SkrCore/platform/vfs.h" // IWYU pragma: export
#include "SkrCore/time.h" // IWYU pragma: export
#include "SkrOS/thread.h" // IWYU pragma: export
#include "SkrBase/math/rtm/rtmx.h" // IWYU pragma: export
#include "SkrBase/misc/make_zeroed.hpp" // IWYU pragma: export

#include "SkrRT/ecs/sugoi.h" // IWYU pragma: export
#include "SkrRT/ecs/type_builder.hpp" // IWYU pragma: export

#include "SkrCore/log.h" // IWYU pragma: export
#include "SkrCore/log.hpp" // IWYU pragma: export

#include <SkrContainers/concurrent_queue.hpp> // IWYU pragma: export
#include <SkrContainers/sptr.hpp> // IWYU pragma: export
#include <SkrContainers/string.hpp> // IWYU pragma: export
#include <SkrContainers/vector.hpp> // IWYU pragma: export
#include <SkrContainers/hashmap.hpp> // IWYU pragma: export

#include <SkrContainers/stl_vector.hpp> // IWYU pragma: export
#include <SkrContainers/stl_deque.hpp> // IWYU pragma: export
#include <SkrContainers/stl_string.hpp> // IWYU pragma: export

#include "SkrRenderGraph/frontend/render_graph.hpp" // IWYU pragma: export

#include "CubismDefaultParameterId.hpp" // IWYU pragma: export
#include "CubismCdiJson.hpp" // IWYU pragma: export

#include "Framework/CubismFramework.hpp" // IWYU pragma: export
#include "Framework/Math/CubismMatrix44.hpp"  // IWYU pragma: export
#include "Framework/Math/CubismViewMatrix.hpp"  // IWYU pragma: export
#include "Framework/Math/CubismVector2.hpp" // IWYU pragma: export
#include "Framework/Utils/CubismDebug.hpp" // IWYU pragma: export
#include "Framework/Type/csmMap.hpp" // IWYU pragma: export
#include "Framework/Effect/CubismBreath.hpp" // IWYU pragma: export
#include "Framework/Model/CubismModel.hpp" // IWYU pragma: export
#include "Framework/Model/CubismUserModel.hpp" // IWYU pragma: export
#include "Framework/CubismModelSettingJson.hpp" // IWYU pragma: export
#include "Framework/Effect/CubismEyeBlink.hpp" // IWYU pragma: export
#include "Framework/Effect/CubismBreath.hpp" // IWYU pragma: export
#include "Framework/Motion/CubismMotion.hpp" // IWYU pragma: export

#include "Utils/CubismString.hpp" // IWYU pragma: export
#include "Utils/CubismDebug.hpp" // IWYU pragma: export
#include "Utils/CubismJson.hpp" // IWYU pragma: export
#include "Id/CubismId.hpp" // IWYU pragma: export
#include "Id/CubismIdManager.hpp" // IWYU pragma: export
#include "Type/csmRectF.hpp" // IWYU pragma: export
#include "Type/csmVector.hpp" // IWYU pragma: export
#include "Type/csmString.hpp" // IWYU pragma: export

#include "Rendering/CubismRenderer.hpp" // IWYU pragma: export

#include "live2d_helpers.hpp" // IWYU pragma: export

#include "SkrProfile/profile.h" // IWYU pragma: export