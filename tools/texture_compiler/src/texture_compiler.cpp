#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrToolCore/project/project.hpp"
#include "dxt_utils.hpp"
#include "io/io.h"
#include "misc/log.hpp"

#include "tracy/Tracy.hpp"

namespace skd
{
namespace asset
{
struct skr_uncompressed_render_texture_t
{
    skr_uncompressed_render_texture_t(skr_image_coder_id coder)
        : image_coder(coder)
    {

    }
    ~skr_uncompressed_render_texture_t()
    {
        if (image_coder) skr_image_coder_free_image(image_coder);
    }
    skr_image_coder_id image_coder = nullptr;
};

void* STextureImporter::Import(skr_io_ram_service_t* ioService, SCookContext* context)
{
    skr_async_ram_destination_t ioDestination = {};
    {
        ZoneScopedN("LoadFileDependencies");
        context->AddFileDependencyAndLoad(ioService, assetPath.c_str(), ioDestination);
    }

    {
        ZoneScopedN("TryDecodeTexture");
        // try decode texture
        const auto uncompressed_data = ioDestination.bytes;
        const auto uncompressed_size = ioDestination.size;
        EImageCoderFormat format = skr_image_coder_detect_format((const uint8_t*)uncompressed_data, uncompressed_size);
        if (auto coder = skr_image_coder_create_image(format))
        {
            bool success = skr_image_coder_move_encoded(coder, uncompressed_data, uncompressed_size);
            if (success)
            {
                return SkrNew<skr_uncompressed_render_texture_t>(coder);
            }
        }
        sakura_free(ioDestination.bytes);
    }

    return nullptr;
}

void STextureImporter::Destroy(void *resource)
{
    SkrDelete((skr_uncompressed_render_texture_t*)resource);
}

bool STextureCooker::Cook(SCookContext *ctx)
{
    const auto outputPath = ctx->GetOutputPath();
    auto uncompressed = ctx->Import<skr_uncompressed_render_texture_t>();
    SKR_DEFER({ ctx->Destroy(uncompressed); });
    
    // try decode texture & calculate compressed format
    const auto image_coder = uncompressed->image_coder;
    const auto format = skr_image_coder_get_color_format(image_coder);
    ECGPUFormat compressed_format = CGPU_FORMAT_UNDEFINED;
    switch (format) // TODO: format shuffle
    {
        case IMAGE_CODER_COLOR_FORMAT_Gray: 
        case IMAGE_CODER_COLOR_FORMAT_GrayF:
            compressed_format = CGPU_FORMAT_DXBC4_UNORM; 
            break;
        case IMAGE_CODER_COLOR_FORMAT_RGBA:
        default:
            compressed_format = CGPU_FORMAT_DXBC3_UNORM; 
            break;
    }
    // DXT
    skr::vector<uint8_t> compressed_data;
    {
        ZoneScopedN("DXTCompress");
        compressed_data = Util_DXTCompressWithImageCoder(image_coder, compressed_format);
    }
    // TODO: ASTC
    // write texture resource
    skr_texture_resource_t resource;
    resource.format = compressed_format;
    resource.mips_count = 1;
    resource.data_size = compressed_data.size();
    resource.height = skr_image_coder_get_height(image_coder);
    resource.width = skr_image_coder_get_width(image_coder);
    resource.depth = 1;
    {
        ZoneScopedN("SaveToCtx");
        if(!ctx->Save(resource))
            return false;
    }
    // write compressed files
    {
        ZoneScopedN("SaveToDisk");

        auto extension = Util_CompressedTypeString(compressed_format);
        auto compressed_path = outputPath;
        compressed_path.replace_extension(extension.c_str());
        auto compressed_pathstr = compressed_path.string();
        auto compressed_file = fopen(compressed_pathstr.c_str(), "wb");
        SKR_DEFER({ fclose(compressed_file); });
        fwrite(compressed_data.data(), compressed_data.size(), 1, compressed_file);
    }
    return true;
}

uint32_t STextureCooker::Version()
{
    return kDevelopmentVersion;
}

} // namespace asset
} // namespace skd