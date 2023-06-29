#include "backend/text_server/image_texture.h"
#include "backend/text_server/text_server.h"

namespace godot
{
// skr::gui::EGDIImageFormat translate_format(ImageFormat format)
// {
//     switch (format)
//     {
//         case ImageFormat::FORMAT_RGB8:
//             return skr::gui::EGDIImageFormat::RGB8;
//         case ImageFormat::FORMAT_RGBA8:
//             return skr::gui::EGDIImageFormat::RGBA8;
//         case ImageFormat::FORMAT_LA8:
//             return skr::gui::EGDIImageFormat::LA8;
//         case ImageFormat::FORMAT_R8:
//             return skr::gui::EGDIImageFormat::R8;
//         default:
//             break;
//     }
//     SKR_UNREACHABLE_CODE();
//     return skr::gui::EGDIImageFormat::None;
// }

// ImageFormat translate_format(skr::gui::EGDIImageFormat format)
// {
//     switch (format)
//     {
//         case skr::gui::EGDIImageFormat::RGB8:
//             return ImageFormat::FORMAT_RGB8;
//         case skr::gui::EGDIImageFormat::RGBA8:
//             return ImageFormat::FORMAT_RGBA8;
//         case skr::gui::EGDIImageFormat::LA8:
//             return ImageFormat::FORMAT_LA8;
//         case skr::gui::EGDIImageFormat::R8:
//             return ImageFormat::FORMAT_R8;
//         default:
//             break;
//     }
//     SKR_UNREACHABLE_CODE();
//     return ImageFormat::FORMAT_None;
// }

// Ref<FontAtlasImage> Image::create_from_data(skr::gui::IGDIRenderer* renderer, uint32_t w, uint32_t h,
//                                             bool p_use_mipmaps, Format format, const Span<const uint8_t>& p_data)
// {
//     Ref<FontAtlasImage> image;
//     image.instantiate();
//     skr::gui::GDIImageDescriptor desc = {};
//     desc.format = translate_format(format);
//     desc.source = skr::gui::EGDIImageSource::Data;
//     desc.from_data.data = p_data.data();
//     desc.from_data.size = p_data.size();
//     desc.from_data.w = w;
//     desc.from_data.h = h;
//     // TODO
//     // desc.from_data.mip_count = p_use_mipmaps ? 0 : 1;
//     image->underlying = renderer->create_image(&desc);
//     return image;
// }

// void Image::generate_mipmaps()
// {
//     auto renderer = underlying->get_renderer();
//     if (renderer->support_mipmap_generation())
//     {
//         SKR_UNIMPLEMENTED_FUNCTION();
//         // use renderer implementation...
//     }
//     else
//     {
//         // use our implementation...
//         SKR_UNIMPLEMENTED_FUNCTION();
//     }
// }

// Image::~Image()
// {
//     if (underlying)
//     {
//         underlying->get_renderer()->free_image(underlying);
//     }
// }

// Span<const uint8_t> Image::get_data()
// {
//     const auto data = underlying->get_data();
//     return { data.data(), data.size() };
// }

// uint32_t    Image::get_width() const { return underlying->get_width(); }
// uint32_t    Image::get_height() const { return underlying->get_height(); }
// ImageFormat Image::get_format() const
// {
//     const auto format = underlying->get_format();
//     return translate_format(format);
// }

// RID_PtrOwner<ImageTexture> ImageTexture::texture_owner = {};
// ImageTexture::~ImageTexture()
// {
//     if (underlying)
//     {
//         // underlying->get_renderer()->free_texture(underlying);
//     }
// }

// Ref<ImageTexture> ImageTexture::create_from_image(skr::gui::IGDIRenderer* renderer, Ref<FontAtlasImage> image)
// {
//     if (auto gdi_image = image.get()->underlying)
//     {
//         while (gdi_image->get_state() != skr::gui::EGDIResourceState::Okay)
//         {
//         }
//         Ref<ImageTexture> texture;
//         texture.instantiate();
//         skr::gui::GDITextureDescriptor desc = {};
//         desc.format = translate_format(image->get_format());
//         desc.source = skr::gui::EGDITextureSource::Image;
//         desc.from_image.image = gdi_image;
//         // texture->underlying = renderer->create_texture(&desc);
//         texture->rid = texture_owner.make_rid(texture.get());
//         return texture;
//     }
//     SKR_ASSERT(0 && "Image is not ready!");
//     return nullptr;
// }

// Size2 ImageTexture::get_size() const
// {
//     return {};
//     // return { (float)underlying->get_width(), (float)underlying->get_height() };
// }

// void ImageTexture::update(const Ref<FontAtlasImage> image)
// {
//     // if (auto texture = underlying)
//     // {
//     //     auto renderer = texture->get_renderer();
//     //     for (auto [image, update] : updates)
//     //     {
//     //         if (update->get_state() == skr::gui::EGDIResourceState::Okay)
//     //         {
//     //             renderer->free_texture_update(update);
//     //             image.reset();
//     //         }
//     //     }
//     //     updates.erase(std::remove_if(updates.begin(), updates.end(), [](auto& pair) { return !pair.first; }), updates.end());
//     //     skr::gui::GDITextureUpdateDescriptor update_desc = {};
//     //     update_desc.texture = texture;
//     //     update_desc.image = image->underlying;
//     //     auto update_handle = renderer->update_texture(&update_desc);
//     //     updates.emplace_back(image, update_handle);
//     // }
// }

// RID ImageTexture::get_rid() const { return rid; }
} // namespace godot

namespace godot
{
FontAtlasImage::FontAtlasImage()
{
    _rid = texture_owner().make_rid(this);
}
FontAtlasImage::~FontAtlasImage()
{
    if (_image) TS->get_resource_service()->destroy_resource(skr::make_not_null(_image));
}

Ref<FontAtlasImage> FontAtlasImage::create(Sizei size, int32_t mip_count, Format format, Span<const uint8_t> data)
{
    Ref<FontAtlasImage> result;

    result.instantiate();
    result->set_size(size);
    result->set_format(format);
    result->set_mip_count(mip_count);
    result->set_data(data);

    return result;
}
Ref<FontAtlasImage> FontAtlasImage::create(Sizei size, int32_t mip_count, Format format)
{
    Ref<FontAtlasImage> result;

    result.instantiate();
    result->set_size(size);
    result->set_format(format);
    result->set_mip_count(mip_count);

    return result;
}

void FontAtlasImage::flush_update() SKR_NOEXCEPT
{
    if (_dirty)
    {
        skr::gui::UpdatableImageDesc desc = {};
        desc.format = _format;
        desc.size = _size;
        desc.mip_count = 0; // TODO. mip maps
        desc.data = { _data.data(), _data.size() };

        if (_image == nullptr)
        {
            _image = TS->get_resource_service()->create_updatable_image(desc);
        }
        else
        {
            _image->update(desc);
        }
        _dirty = false;
    }
}

// RID
RID_PtrOwner<FontAtlasImage>& FontAtlasImage::texture_owner()
{
    static RID_PtrOwner<FontAtlasImage> _owner = {};
    return _owner;
}

} // namespace godot