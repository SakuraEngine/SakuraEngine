#include "kernel.ispc.h"
#include "SkrTextureCompiler/texture_compiler.hpp"
#include "skr_renderer/render_texture.h"
#include "utils/log.hpp"

namespace skd
{
namespace asset
{
void* STextureImporter::Import(skr::io::RAMService*, SCookContext *context)
{
    return SkrNew<skr_render_texture_t>();
}

void STextureImporter::Destroy(void *resource)
{
    SkrDelete((skr_render_texture_t*)resource);
}

bool STextureCooker::Cook(SCookContext *ctx)
{
    auto world = ctx->Import<skr_render_texture_t>();
    SKR_DEFER({ ctx->Destroy(world); });

    eastl::vector<uint8_t> buffer = {1, 1, 1, 1};
    auto file = fopen(ctx->output.u8string().c_str(), "wb");
    if (!file)
    {
        SKR_LOG_FMT_ERROR("[SConfigCooker::Cook] failed to write cooked file for resource {}! path: {}", ctx->record->guid, ctx->record->path.u8string());
        return false;
    }
    SKR_DEFER({ fclose(file); });
    fwrite(buffer.data(), 1, buffer.size(), file);

    return true;
}

uint32_t STextureCooker::Version()
{
    return 0;
}

bool STextureImporterFactory::CanImport(const SAssetRecord *record)
{
    return true;
}

skr_guid_t STextureImporterFactory::GetResourceType()
{
    return skr_guid_t();
}

void STextureImporterFactory::CreateImporter(const SAssetRecord *record)
{

}

} // namespace asset
} // namespace skd