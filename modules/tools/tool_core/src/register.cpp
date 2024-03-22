#include "SkrToolCore/assets/config_asset.hpp"

#include "SkrRT/resource/config_resource.h"

struct _ToolCoreRegister {
    _ToolCoreRegister()
    {
#define _DEFAULT_COOKER(__COOKER_TYPE, __RESOURCE_TYPE) skd::asset::RegisterCooker<__COOKER_TYPE>(true, skr::rttr::RTTRTraits<__COOKER_TYPE>::get_guid(), skr::rttr::RTTRTraits<__RESOURCE_TYPE>::get_guid());
        _DEFAULT_COOKER(skd::asset::SConfigCooker, skr_config_resource_t)
#undef _DEFAULT_COOKER

#define _IMPORTER(__TYPE) skd::asset::RegisterImporter<__TYPE>(skr::rttr::RTTRTraits<__TYPE>::get_guid());
        _IMPORTER(skd::asset::SJsonConfigImporter)
#undef _IMPORTER
    }
} _tool_core_register;