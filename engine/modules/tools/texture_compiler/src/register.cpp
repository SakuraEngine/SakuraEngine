#include "SkrTextureCompiler/texture_compiler.hpp"
#include "SkrTextureCompiler/texture_sampler_asset.hpp"

#include "SkrRenderer/resources/texture_resource.h"
#include "SkrRenderer/resources/texture_resource.h"

struct _TextureCompilerRegister
{
    _TextureCompilerRegister()
    {
#define _DEFAULT_COOKER(__COOKER_TYPE, __RESOURCE_TYPE) skd::asset::RegisterCooker<__COOKER_TYPE>(true, skr::TypeInfo<__COOKER_TYPE>::get_guid(), skr::TypeInfo<__RESOURCE_TYPE>::get_guid());
        _DEFAULT_COOKER(skd::asset::TextureCooker, skr::TextureResource)
        _DEFAULT_COOKER(skd::asset::TextureSamplerCooker, skr::TextureSamplerResource)
#undef _DEFAULT_COOKER

#define _IMPORTER(__TYPE) skd::asset::RegisterImporter<__TYPE>(skr::TypeInfo<__TYPE>::get_guid());
        _IMPORTER(skd::asset::TextureImporter)
        _IMPORTER(skd::asset::TextureSamplerImporter)
#undef _IMPORTER
    }
} _texture_compiler_register;