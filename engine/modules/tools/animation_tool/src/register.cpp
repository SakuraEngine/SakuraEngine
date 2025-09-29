#include "SkrAnim/resources/animation_resource.hpp"
#include "SkrAnim/resources/skin_resource.hpp"

#include "SkrAnimTool/animation_asset.h"
#include "SkrAnimTool/skeleton_asset.h"
#include "SkrAnimTool/skin_asset.h"

struct _AnimationToolRegister
{
    _AnimationToolRegister()
    {
#define _DEFAULT_COOKER(__COOKER_TYPE, __RESOURCE_TYPE) skr::RegisterCooker<__COOKER_TYPE>(true, skr::TypeInfo<__COOKER_TYPE>::get_guid(), skr::TypeInfo<__RESOURCE_TYPE>::get_guid());
        _DEFAULT_COOKER(skr::AnimCooker, skr::AnimResource)
        _DEFAULT_COOKER(skr::SkelCooker, skr::SkeletonResource)
        _DEFAULT_COOKER(skr::SkinCooker, skr::SkinResource)
#undef _DEFAULT_COOKER

#define _IMPORTER(__TYPE) skr::RegisterImporter<__TYPE>(skr::TypeInfo<__TYPE>::get_guid());
        _IMPORTER(skr::GltfAnimImporter)
        _IMPORTER(skr::GltfSkelImporter)
#undef _IMPORTER
    }
} _animation_tool_register;