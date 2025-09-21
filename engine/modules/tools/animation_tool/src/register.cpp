#include "SkrAnim/resources/animation_resource.hpp"
#include "SkrAnim/resources/skin_resource.hpp"

#include "SkrAnimTool/animation_asset.h"
#include "SkrAnimTool/skeleton_asset.h"
#include "SkrAnimTool/skin_asset.h"

struct _AnimationToolRegister
{
    _AnimationToolRegister()
    {
#define _DEFAULT_COOKER(__COOKER_TYPE, __RESOURCE_TYPE) skd::asset::RegisterCooker<__COOKER_TYPE>(true, skr::TypeInfo<__COOKER_TYPE>::get_guid(), skr::TypeInfo<__RESOURCE_TYPE>::get_guid());
        _DEFAULT_COOKER(skd::asset::AnimCooker, skr::AnimResource)
        _DEFAULT_COOKER(skd::asset::SkelCooker, skr::SkeletonResource)
        _DEFAULT_COOKER(skd::asset::SkinCooker, skr::SkinResource)
#undef _DEFAULT_COOKER

#define _IMPORTER(__TYPE) skd::asset::RegisterImporter<__TYPE>(skr::TypeInfo<__TYPE>::get_guid());
        _IMPORTER(skd::asset::GltfAnimImporter)
        _IMPORTER(skd::asset::GltfSkelImporter)
#undef _IMPORTER
    }
} _animation_tool_register;