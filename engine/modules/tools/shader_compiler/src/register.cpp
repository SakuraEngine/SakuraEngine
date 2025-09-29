#include "SkrShaderCompiler/assets/material_asset.hpp"
#include "SkrShaderCompiler/assets/material_type_asset.hpp"
#include "SkrShaderCompiler/assets/shader_asset.hpp"

#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"
#include "SkrRenderer/resources/shader_resource.hpp"

struct _ShaderCompilerRegister {
    _ShaderCompilerRegister()
    {
#define _DEFAULT_COOKER(__COOKER_TYPE, __RESOURCE_TYPE) skr::RegisterCooker<__COOKER_TYPE>(true, skr::TypeInfo<__COOKER_TYPE>::get_guid(), skr::TypeInfo<__RESOURCE_TYPE>::get_guid());
        _DEFAULT_COOKER(skr::MaterialCooker, skr::MaterialResource)
        _DEFAULT_COOKER(skr::MaterialTypeCooker, skr::MaterialTypeResource)
        _DEFAULT_COOKER(skr::ShaderCooker, skr::ShaderCollectionResource)
        _DEFAULT_COOKER(skr::ShaderOptionsCooker, skr::ShaderOptionsResource)
#undef _DEFAULT_COOKER

#define _IMPORTER(__TYPE) skr::RegisterImporter<__TYPE>(skr::TypeInfo<__TYPE>::get_guid());
        _IMPORTER(skr::MaterialImporter)
        _IMPORTER(skr::MaterialTypeImporter)
        _IMPORTER(skr::ShaderOptionImporter)
        _IMPORTER(skr::ShaderImporter)
#undef _IMPORTER
    }
} _shader_compiler_register;