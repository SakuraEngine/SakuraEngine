#pragma once
#include "platform/configure.h"
#include "SkrGLTFTool/module.configure.h"
#include "cgltf/cgltf.h"
#include "utils/io.hpp"
#include <containers/string.hpp>

namespace skd
{
namespace asset
{

// returned cgltf_data* needs to be freed by cgltf_free
GLTFTOOL_API GLTFTOOL_EXTERN_C
cgltf_data* ImportGLTFWithData(skr::string_view assetPath, skr::io::RAMService* ioService, struct SCookContext* context) SKR_NOEXCEPT;

// LUT for gltf attributes to semantic names
static const char* kGLTFAttributeTypeLUT[8] = {
    "NONE",
    "POSITION",
    "NORMAL",
    "TANGENT",
    "TEXCOORD",
    "COLOR",
    "JOINTS",
    "WEIGHTS"
}; 

} // namespace asset
} // namespace skd