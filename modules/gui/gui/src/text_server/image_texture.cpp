#include "text_server/image_texture.h"

namespace godot {
Ref<Image> Image::create_from_data(skr::gdi::SGDIRenderer* renderer, uint32_t w, uint32_t h, bool p_use_mipmaps, Format format, const Vector<uint8_t> &p_data)
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return {};
}

void Image::generate_mipmaps() { SKR_UNIMPLEMENTED_FUNCTION(); }
Span<uint8_t> Image::get_data() { SKR_UNIMPLEMENTED_FUNCTION(); return {}; }
uint32_t Image::get_width() const { SKR_UNIMPLEMENTED_FUNCTION(); return UINT32_MAX; }
uint32_t Image::get_height() const { SKR_UNIMPLEMENTED_FUNCTION(); return UINT32_MAX; }
ImageFormat Image::get_format() const { SKR_UNIMPLEMENTED_FUNCTION(); return FORMAT_COUNT; }

Ref<ImageTexture> ImageTexture::create_from_image(skr::gdi::SGDIRenderer* renderer,Ref<Image> image)
{
    return {};
}
Size2 ImageTexture::get_size() const { SKR_UNIMPLEMENTED_FUNCTION(); return {};}
void ImageTexture::update(const Ref<Image> image) { SKR_UNIMPLEMENTED_FUNCTION(); }
RID ImageTexture::get_rid() const { SKR_UNIMPLEMENTED_FUNCTION(); return {}; }
}