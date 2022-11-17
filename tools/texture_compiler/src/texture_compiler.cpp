#include "dxt_utils.hpp"
#include "utils/io.hpp"
#include "utils/log.hpp"

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

void* STextureImporter::Import(skr::io::RAMService* ioService, SCookContext* context)
{
    auto path = context->AddFileDependency(assetPath.c_str());
    auto u8Path = path.u8string();
    const auto assetRecord = context->GetAssetRecord();

    // load file
    skr::task::event_t counter;
    skr_ram_io_t ramIO = {};
    ramIO.offset = 0;
    ramIO.path = u8Path.c_str();
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request,void* data) noexcept {
        auto pCounter = (skr::task::event_t*)data;
        pCounter->signal();
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)&counter;
    skr_async_request_t ioRequest = {};
    skr_async_ram_destination_t ioDestination = {};
    ioService->request(assetRecord->project->vfs, &ramIO, &ioRequest, &ioDestination);
    counter.wait(false);

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
    sakura_free(uncompressed_data);
    return nullptr;
}

void STextureImporter::Destroy(void *resource)
{
    SkrDelete((skr_uncompressed_render_texture_t*)resource);
}

bool STextureCooker::Cook(SCookContext *ctx)
{
    const auto outputPath = ctx->GetOutputPath();
    const auto assetRecord = ctx->GetAssetRecord();
    auto uncompressed = ctx->Import<skr_uncompressed_render_texture_t>();
    SKR_DEFER({ ctx->Destroy(uncompressed); });
    
    // try decode texture & calculate compressed format
    const auto image_coder = uncompressed->image_coder;
    const auto format = skr_image_coder_get_color_format(image_coder);
    ECGPUFormat compressed_format = CGPU_FORMAT_UNDEFINED;
    switch (format)
    {
        case IMAGE_CODER_COLOR_FORMAT_RGBA: // TODO: format shuffle
        default:
            compressed_format = CGPU_FORMAT_DXBC1_RGBA_UNORM; // TODO: determine format
            break;
    }
    // DXT
    const auto compressed_data = Util_DXTCompressWithImageCoder(image_coder, compressed_format);
    // TODO: ASTC
    // make archive
    eastl::vector<uint8_t> resource_data;
    struct VectorWriter
    {
        eastl::vector<uint8_t>* buffer;
        int write(const void* data, size_t size)
        {
            buffer->insert(buffer->end(), (uint8_t*)data, (uint8_t*)data + size);
            return 0;
        }
    } writer{&resource_data};
    skr_binary_writer_t archive(writer);
    // write texture resource
    skr_texture_resource_t resource;
    resource.format = compressed_format;
    resource.mips_count = 1;
    resource.data_size = compressed_data.size();
    resource.height = skr_image_coder_get_height(image_coder);
    resource.width = skr_image_coder_get_width(image_coder);
    resource.depth = 1;
    // format
    skr::binary::Write(&archive, resource);
    // write to file
    auto file = fopen(outputPath.u8string().c_str(), "wb");
    if (!file)
    {
        SKR_LOG_FMT_ERROR("[SConfigCooker::Cook] failed to write cooked file for resource {}! path: {}", 
            assetRecord->guid, assetRecord->path.u8string());
        return false;
    }
    SKR_DEFER({ fclose(file); });
    fwrite(resource_data.data(), resource_data.size(), 1, file);
    // write compressed files
    {
        auto extension = Util_CompressedTypeString(compressed_format);
        auto compressed_path = outputPath;
        compressed_path.replace_extension(extension.c_str());
        auto compressed_file = fopen(compressed_path.u8string().c_str(), "wb");
        SKR_DEFER({ fclose(compressed_file); });
        fwrite(compressed_data.data(), compressed_data.size(), 1, compressed_file);
    }
    return true;
}

uint32_t STextureCooker::Version()
{
    return kDevelopmentVersion;
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