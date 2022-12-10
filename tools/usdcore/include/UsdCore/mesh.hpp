#pragma once
#include "UsdCore/prim.hpp"
#include "cgltf/cgltf.h"

namespace skd
{
    USDCORE_API cgltf_data* USDCoreConvertToGLTF(const SUSDPrimId& prim);
}