#include "backend/text_server/image_texture.h"
#include "backend/text_server/text_server.h"
#include "SkrGui/backend/device/device.hpp"

namespace godot
{
FontAtlasImage::FontAtlasImage()
{
    _rid = texture_owner().make_rid(this);
}
FontAtlasImage::~FontAtlasImage()
{
    if (_image) _image->destroy();
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
        desc.format                       = _format;
        desc.size                         = _size;
        desc.mip_count                    = 0; // TODO. mip maps
        desc.data                         = { _data.data(), _data.size() };

        if (_image == nullptr)
        {
            _image = TS->get_resource_service()->create_updatable_image();
        }

        if (desc.data.size() > 0 && desc.size != Sizei::Zero())
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