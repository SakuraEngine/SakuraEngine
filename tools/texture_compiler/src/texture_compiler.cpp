#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrToolCore/project/project.hpp"
#include "dxt_utils.hpp"
#include "SkrRT/io/ram_io.hpp"
#include "SkrRT/misc/log.hpp"

#include "SkrProfile/profile.h"

namespace skd
{
namespace asset
{
struct skr_uncompressed_render_texture_t
{
    skr_uncompressed_render_texture_t(skr::ImageDecoderId decoder, skr_io_ram_service_t* ioService, skr::BlobId blob)
        : decoder(decoder), ioService(ioService), blob(blob)
    {
        decoder->initialize(blob->get_data(), blob->get_size());
    }

    ~skr_uncompressed_render_texture_t()
    {
        if (decoder)
        {
            blob.reset();
            decoder.reset();
        }
    }

    skr::ImageDecoderId decoder = nullptr;
    skr_io_ram_service_t* ioService = nullptr;
    skr::BlobId blob = nullptr;
};

void* STextureImporter::Import(skr_io_ram_service_t* ioService, SCookContext* context)
{
    skr::BlobId blob = nullptr;
    {
        SkrZoneScopedN("LoadFileDependencies");
        context->AddFileDependencyAndLoad(ioService, assetPath.c_str(), blob);
    }

    {
        SkrZoneScopedN("TryDecodeTexture");
        // try decode texture
        const auto uncompressed_data = blob->get_data();
        const auto uncompressed_size = blob->get_size();
        EImageCoderFormat format = skr_image_coder_detect_format((const uint8_t*)uncompressed_data, uncompressed_size);
        if (auto decoder = skr::IImageDecoder::Create(format))
        {
            return SkrNew<skr_uncompressed_render_texture_t>(decoder, ioService, blob);
        }
        else
        {
            SKR_DEFER({blob.reset();});
        }
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
    const auto decoder = uncompressed->decoder;
    const auto format = decoder->get_color_format();
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
        SkrZoneScopedN("DXTCompress");
        compressed_data = Util_DXTCompressWithImageCoder(decoder, compressed_format);
    }
    // TODO: ASTC
    // write texture resource
    skr_texture_resource_t resource;
    resource.format = compressed_format;
    resource.mips_count = 1;
    resource.data_size = compressed_data.size();
    resource.height = decoder->get_height();
    resource.width = decoder->get_width();
    resource.depth = 1;
    {
        SkrZoneScopedN("SaveToCtx");
        if(!ctx->Save(resource))
            return false;
    }
    // write compressed files
    {
        SkrZoneScopedN("SaveToDisk");

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