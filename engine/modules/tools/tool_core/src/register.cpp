#include "SkrToolCore/assets/config_asset.hpp"

#include "SkrRuntime/resource/config_resource.h"

struct _ToolCoreRegister
{
    _ToolCoreRegister()
    {
#define _DEFAULT_COOKER(__COOKER_TYPE, __RESOURCE_TYPE) skd::asset::RegisterCooker<__COOKER_TYPE>(true, skr::TypeInfo<__COOKER_TYPE>::get_guid(), skr::TypeInfo<__RESOURCE_TYPE>::get_guid());
        _DEFAULT_COOKER(skd::asset::ConfigCooker, skr_config_resource_t)
#undef _DEFAULT_COOKER

#define _IMPORTER(__TYPE) skd::asset::RegisterImporter<__TYPE>(skr::TypeInfo<__TYPE>::get_guid());
        _IMPORTER(skd::asset::JsonConfigImporter)
#undef _IMPORTER
    }
} _tool_core_register;