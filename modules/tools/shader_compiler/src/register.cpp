#include "SkrShaderCompiler/assets/material_asset.hpp"
#include "SkrShaderCompiler/assets/material_type_asset.hpp"
#include "SkrShaderCompiler/assets/shader_asset.hpp"

#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"
#include "SkrRenderer/resources/shader_resource.hpp"

struct _ShaderCompilerRegister {
    _ShaderCompilerRegister()
    {
#define _DEFAULT_COOKER(__COOKER_TYPE, __RESOURCE_TYPE) skd::asset::RegisterCooker<__COOKER_TYPE>(true, skr::rttr::RTTRTraits<__COOKER_TYPE>::get_guid(), skr::rttr::RTTRTraits<__RESOURCE_TYPE>::get_guid());
        _DEFAULT_COOKER(skd::asset::SMaterialCooker, skr::renderer::MaterialResource)
        _DEFAULT_COOKER(skd::asset::SMaterialTypeCooker, skr::renderer::MaterialTypeResource)
        _DEFAULT_COOKER(skd::asset::SShaderCooker, skr::renderer::ShaderCollectionResource)
        _DEFAULT_COOKER(skd::asset::SShaderOptionsCooker, skr::renderer::ShaderOptionsResource)
#undef _DEFAULT_COOKER

#define _IMPORTER(__TYPE) skd::asset::RegisterImporter<__TYPE>(skr::rttr::RTTRTraits<__TYPE>::get_guid());
        _IMPORTER(skd::asset::SMaterialImporter)
        _IMPORTER(skd::asset::SMaterialTypeImporter)
        _IMPORTER(skd::asset::SShaderOptionsImporter)
        _IMPORTER(skd::asset::SShaderImporter)
#undef _IMPORTER
    }
} _shader_compiler_register;