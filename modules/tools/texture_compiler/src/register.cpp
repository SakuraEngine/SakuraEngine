#include "SkrTextureCompiler/texture_compiler.hpp"
#include "SkrTextureCompiler/texture_sampler_asset.hpp"

#include "SkrRenderer/resources/texture_resource.h"
#include "SkrRenderer/resources/texture_resource.h"

struct _TextureCompilerRegister {
    _TextureCompilerRegister()
    {
#define _DEFAULT_COOKER(__COOKER_TYPE, __RESOURCE_TYPE) skd::asset::RegisterCooker<__COOKER_TYPE>(true, skr::rttr::RTTRTraits<__COOKER_TYPE>::get_guid(), skr::rttr::RTTRTraits<__RESOURCE_TYPE>::get_guid());
        _DEFAULT_COOKER(skd::asset::STextureCooker, skr_texture_resource_t)
        _DEFAULT_COOKER(skd::asset::STextureSamplerCooker, skr_texture_sampler_resource_t)
#undef _DEFAULT_COOKER

#define _IMPORTER(__TYPE) skd::asset::RegisterImporter<__TYPE>(skr::rttr::RTTRTraits<__TYPE>::get_guid());
        _IMPORTER(skd::asset::STextureImporter)
        _IMPORTER(skd::asset::STextureSamplerImporter)
#undef _IMPORTER
    }
} _texture_compiler_register;