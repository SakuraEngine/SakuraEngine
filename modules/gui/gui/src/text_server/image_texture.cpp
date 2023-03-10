#include "text_server/image_texture.h"
#include "SkrGui/interface/gdi_renderer.hpp"

namespace godot {
skr::gdi::EGDIImageFormat translate_format(ImageFormat format)
{
    switch (format)
    {
        case ImageFormat::FORMAT_RGB8: return skr::gdi::EGDIImageFormat::RGB8;
        case ImageFormat::FORMAT_RGBA8: return skr::gdi::EGDIImageFormat::RGBA8;
        case ImageFormat::FORMAT_LA8: return skr::gdi::EGDIImageFormat::LA8;
        case ImageFormat::FORMAT_R8: return skr::gdi::EGDIImageFormat::R8;
        default: break;
    }
    SKR_UNREACHABLE_CODE();
    return skr::gdi::EGDIImageFormat::None;
}
ImageFormat translate_format(skr::gdi::EGDIImageFormat format)
{
    switch (format)
    {
        case skr::gdi::EGDIImageFormat::RGB8: return ImageFormat::FORMAT_RGB8;
        case skr::gdi::EGDIImageFormat::RGBA8: return ImageFormat::FORMAT_RGBA8;
        case skr::gdi::EGDIImageFormat::LA8: return ImageFormat::FORMAT_LA8;
        case skr::gdi::EGDIImageFormat::R8: return ImageFormat::FORMAT_R8;
        default: break;
    }
    SKR_UNREACHABLE_CODE();
    return ImageFormat::FORMAT_None;
}

Ref<Image> Image::create_from_data(skr::gdi::IGDIRenderer* renderer, uint32_t w, uint32_t h, 
    bool p_use_mipmaps, Format format, const Span<const uint8_t> &p_data)
{
    Ref<Image> image;
    image.instantiate();
    skr::gdi::SGDIImageDescriptor desc = {};
    desc.format = translate_format(format);
    desc.source = skr::gdi::EGDIImageSource::Data;
    desc.from_data.data = p_data.data();
    desc.from_data.size = p_data.size();
    desc.from_data.w = w;
    desc.from_data.h = h;
    // TODO
    // desc.from_data.mip_count = p_use_mipmaps ? 0 : 1;
    image->underlying = renderer->create_image(&desc);
    return image;
}

void Image::generate_mipmaps() 
{
    auto renderer = underlying->get_renderer();
    if (renderer->support_mipmap_generation())
    {
        SKR_UNIMPLEMENTED_FUNCTION(); 
        // use renderer implementation...
    }
    else
    {
        // use our implementation... 
        SKR_UNIMPLEMENTED_FUNCTION(); 
    }
}

Span<const uint8_t> Image::get_data() 
{ 
    const auto data = underlying->get_data();
    return { data.data(), data.size() };
}

uint32_t Image::get_width() const { return underlying->get_width(); }
uint32_t Image::get_height() const { return underlying->get_height(); }
ImageFormat Image::get_format() const 
{
    const auto format = underlying->get_format();
    return translate_format(format);
}

RID_PtrOwner<ImageTexture> ImageTexture::texture_owner = {};
Ref<ImageTexture> ImageTexture::create_from_image(skr::gdi::IGDIRenderer* renderer, Ref<Image> image)
{
    Ref<ImageTexture> texture;
    texture.instantiate();
    skr::gdi::SGDITextureDescriptor desc = {};
    desc.format = translate_format(image->get_format());
    desc.source = skr::gdi::EGDITextureSource::Image;
    desc.from_image.image = image->underlying;
    texture->underlying = renderer->create_texture(&desc);
    texture->rid = texture_owner.make_rid(texture.get());
    return texture;
}

Size2 ImageTexture::get_size() const { return { (float)underlying->get_width(), (float)underlying->get_height() }; }
void ImageTexture::update(const Ref<Image> image) { SKR_UNIMPLEMENTED_FUNCTION(); }
RID ImageTexture::get_rid() const { return rid; }
}