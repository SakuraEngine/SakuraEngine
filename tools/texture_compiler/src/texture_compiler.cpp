#include "SkrTextureCompiler/texture_compiler.hpp"
#include "ispc/ispc_texcomp.h"
#include "skr_renderer/render_texture.h"
#include "utils/log.hpp"
#include "skr_image_coder/skr_image_coder.h"
#include "utils/io.hpp"

#define TEX_COMPRESS_ALIGN(x, a) (((x) + ((a)-1)) & ~((a)-1))

namespace skd
{
namespace asset
{
struct skr_uncompressed_render_texture_t : public skr_render_texture_t
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

void* STextureImporter::Import(skr::io::RAMService* ioService, SCookContext *context)
{
    auto path = context->AddFileDependency(assetPath.c_str());
    auto u8Path = path.u8string();

    // load file
    skr::task::event_t counter;
    skr_ram_io_t ramIO = {};
    ramIO.offset = 0;
    ramIO.path = u8Path.c_str();
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request,void* data) noexcept {
        auto pCounter = (skr::task::event_t*)data;
        pCounter->signal();
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)&counter;
    skr_async_io_request_t ioRequest = {};
    skr_async_ram_destination_t ioDestination = {};
    ioService->request(context->record->project->vfs, &ramIO, &ioRequest, &ioDestination);
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

inline SKR_CONSTEXPR uint64_t Util_DXBCCompressedSize(uint32_t width, uint32_t height, ECGPUFormat format, uint32_t block_width = 4, uint32_t block_height = 4)
{
    const auto alignedW = TEX_COMPRESS_ALIGN(width, 4);
    const auto alignedH = TEX_COMPRESS_ALIGN(height, 4);
    const auto blocksW = alignedW / block_width;
    const auto blocksH = alignedH / block_height;
    switch (format)
    {
    case CGPU_FORMAT_DXBC1_RGB_UNORM:
    case CGPU_FORMAT_DXBC1_RGB_SRGB:
    case CGPU_FORMAT_DXBC1_RGBA_UNORM:
    case CGPU_FORMAT_DXBC1_RGBA_SRGB:
        return (blocksW * blocksH) * 8;
    case CGPU_FORMAT_DXBC3_UNORM:
    case CGPU_FORMAT_DXBC3_SRGB:
        return (blocksW * blocksH) * 16;
    case CGPU_FORMAT_DXBC4_UNORM:
    case CGPU_FORMAT_DXBC4_SNORM:
        return (blocksW * blocksH) * 8;
    case CGPU_FORMAT_DXBC6H_UFLOAT:
    case CGPU_FORMAT_DXBC6H_SFLOAT:
        return (blocksW * blocksH) * 16;
    case CGPU_FORMAT_DXBC7_UNORM:
    case CGPU_FORMAT_DXBC7_SRGB:
        return (blocksW * blocksH) * 16;
    default:
        return 0;
    }
}

bool STextureCooker::Cook(SCookContext *ctx)
{
    auto uncompressed = ctx->Import<skr_uncompressed_render_texture_t>();
    const auto image_coder = uncompressed->image_coder;
    SKR_DEFER({ ctx->Destroy(uncompressed); });
    
    // try decode texture
    const auto image_width = skr_image_coder_get_width(image_coder);
    const auto image_height = skr_image_coder_get_height(image_coder);
    const auto format = skr_image_coder_get_color_format(image_coder);
    const auto bit_depth = image_coder->get_bit_depth();
    const auto encoded_format = image_coder->get_color_format();
    const auto raw_format = (encoded_format == IMAGE_CODER_COLOR_FORMAT_BGRA) ? IMAGE_CODER_COLOR_FORMAT_RGBA : encoded_format;
    ECGPUFormat compressed_format = CGPU_FORMAT_UNDEFINED;
    switch (format)
    {
        case IMAGE_CODER_COLOR_FORMAT_RGBA: // TODO: format shuffle
        default:
            compressed_format = CGPU_FORMAT_DXBC1_RGBA_UNORM;
            break;
    }
    const auto compressed_size = Util_DXBCCompressedSize(image_width, image_height, compressed_format);
    eastl::vector<uint8_t> compressed_data(compressed_size);
    rgba_surface rgba_surface = {};
    uint8_t* rgba_data = nullptr;
    uint64_t rgba_size = 0;
    bool sucess = skr_image_coder_get_raw_data_view(image_coder, &rgba_data, &rgba_size, raw_format, bit_depth);
    if (!sucess)
    {
        SKR_UNREACHABLE_CODE()
        return false;
    }
    rgba_surface.ptr = rgba_data;
    rgba_surface.width = image_width;
    rgba_surface.height = image_height;
    rgba_surface.stride = image_width * 4;
    CompressBlocksBC1(&rgba_surface, compressed_data.data());
    // archive
    eastl::vector<uint8_t> header;
    struct VectorWriter
    {
        eastl::vector<uint8_t>* buffer;
        int write(const void* data, size_t size)
        {
            buffer->insert(buffer->end(), (uint8_t*)data, (uint8_t*)data + size);
            return 0;
        }
    } writer{&header};
    skr_binary_writer_t archive(writer);
    // format
    skr::binary::WriteValue(&archive, (uint32_t)compressed_format);
    // mips count
    skr::binary::WriteValue(&archive, (uint32_t)1);
    // data size
    skr::binary::WriteValue(&archive, (uint64_t)compressed_size);
    // write to file
    auto file = fopen(ctx->output.u8string().c_str(), "wb");
    if (!file)
    {
        SKR_LOG_FMT_ERROR("[SConfigCooker::Cook] failed to write cooked file for resource {}! path: {}", ctx->record->guid, ctx->record->path.u8string());
        return false;
    }
    SKR_DEFER({ fclose(file); });
    fwrite(header.data(), header.size(), 1, file);
    fwrite(compressed_data.data(), compressed_data.size(), 1, file);
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