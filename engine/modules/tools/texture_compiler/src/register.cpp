#include "SkrTextureCompiler/texture_compiler.hpp"
#include "SkrTextureCompiler/texture_sampler_asset.hpp"

#include "SkrRenderer/resources/texture_resource.h"
#include "SkrRenderer/resources/texture_resource.h"

struct _TextureCompilerRegister
{
    _TextureCompilerRegister()
    {
#define _DEFAULT_COOKER(__COOKER_TYPE, __RESOURCE_TYPE) skr::RegisterCooker<__COOKER_TYPE>(true, skr::TypeInfo<__COOKER_TYPE>::get_guid(), skr::TypeInfo<__RESOURCE_TYPE>::get_guid());
        _DEFAULT_COOKER(skr::TextureCooker, skr::TextureResource)
        _DEFAULT_COOKER(skr::TextureSamplerCooker, skr::TextureSamplerResource)
#undef _DEFAULT_COOKER

#define _IMPORTER(__TYPE) skr::RegisterImporter<__TYPE>(skr::TypeInfo<__TYPE>::get_guid());
        _IMPORTER(skr::TextureImporter)
        _IMPORTER(skr::TextureSamplerImporter)
#undef _IMPORTER
    }
} _texture_compiler_register;