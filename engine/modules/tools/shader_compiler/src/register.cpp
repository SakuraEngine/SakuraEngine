#include "SkrShaderCompiler/assets/material_asset.hpp"
#include "SkrShaderCompiler/assets/material_type_asset.hpp"
#include "SkrShaderCompiler/assets/shader_asset.hpp"

#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"
#include "SkrRenderer/resources/shader_resource.hpp"

struct _ShaderCompilerRegister {
    _ShaderCompilerRegister()
    {
#define _DEFAULT_COOKER(__COOKER_TYPE, __RESOURCE_TYPE) skd::asset::RegisterCooker<__COOKER_TYPE>(true, skr::TypeInfo<__COOKER_TYPE>::get_guid(), skr::TypeInfo<__RESOURCE_TYPE>::get_guid());
        _DEFAULT_COOKER(skd::asset::MaterialCooker, skr::MaterialResource)
        _DEFAULT_COOKER(skd::asset::MaterialTypeCooker, skr::MaterialTypeResource)
        _DEFAULT_COOKER(skd::asset::ShaderCooker, skr::ShaderCollectionResource)
        _DEFAULT_COOKER(skd::asset::ShaderOptionsCooker, skr::ShaderOptionsResource)
#undef _DEFAULT_COOKER

#define _IMPORTER(__TYPE) skd::asset::RegisterImporter<__TYPE>(skr::TypeInfo<__TYPE>::get_guid());
        _IMPORTER(skd::asset::MaterialImporter)
        _IMPORTER(skd::asset::MaterialTypeImporter)
        _IMPORTER(skd::asset::ShaderOptionImporter)
        _IMPORTER(skd::asset::ShaderImporter)
#undef _IMPORTER
    }
} _shader_compiler_register;