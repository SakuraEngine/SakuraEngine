#pragma once

#ifdef _WIN32
#include <intrin.h> // IWYU pragma: export
#include <new.h> // IWYU pragma: export
#endif

#include <EASTL/fixed_vector.h> // IWYU pragma: export
#include <EASTL/fixed_string.h> // IWYU pragma: export

#include "SkrBase/misc/debug.h"  // IWYU pragma: export
#include "SkrRT/platform/guid.hpp" // IWYU pragma: export
#include <SkrRT/platform/filesystem.hpp> // IWYU pragma: export
#include "SkrRT/platform/memory.h" // IWYU pragma: export
#include "SkrRT/platform/vfs.h" // IWYU pragma: export
#include "SkrRT/platform/time.h" // IWYU pragma: export
#include "SkrRT/platform/guid.hpp" // IWYU pragma: export
#include "SkrRT/platform/thread.h" // IWYU pragma: export
#include "SkrBase/math/rtm/rtmx.h" // IWYU pragma: export
#include "SkrRT/misc/make_zeroed.hpp" // IWYU pragma: export

#include "SkrRT/ecs/entity.hpp" // IWYU pragma: export
#include "SkrRT/ecs/dual.h" // IWYU pragma: export
#include "SkrRT/ecs/type_builder.hpp" // IWYU pragma: export

#include "SkrRT/misc/log.h" // IWYU pragma: export
#include "SkrRT/misc/log.hpp" // IWYU pragma: export

#include <SkrRT/containers_new/concurrent_queue.h> // IWYU pragma: export
#include <SkrRT/containers_new/sptr.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/string.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/vector.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/hashmap.hpp> // IWYU pragma: export

#include "SkrRenderGraph/frontend/render_graph.hpp" // IWYU pragma: export

// CUBISM HEADERS
/* TODO: MAYBE THIS IS AN XMAKE PCH BUG
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
*/

#include "SkrProfile/profile.h" // IWYU pragma: export