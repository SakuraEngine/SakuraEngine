#pragma once
#include "backend/text_server/vector2.h"
#include "backend/text_server/rid.h"
#include "backend/text_server/rid_owner.h"
#include "backend/text_server/containers.h"
#include "SkrGui/backend/resource/resource.hpp" // TODO. move to cpp
#include "SkrGui/backend/embed_services.hpp"    // TODO. move to cpp

namespace skr::gui
{
struct IGDIRenderer;
struct IGDIImage;
struct ITexture;
struct IGDITextureUpdate;
} // namespace skr::gui

namespace godot
{
using ImageFormat = ::skr::gui::EPixelFormat;

struct FontAtlasImage {
    using Format = ::skr::gui::EPixelFormat;
    using Sizei = ::skr::gui::Sizei;
    using ImageProvider = ::skr::gui::IEmbeddedTextServiceResourceProvider;
    using ImageEntry = ::skr::gui::IUpdatableImageEntry;
    using Image = ::skr::gui::IImage;

    FontAtlasImage();
    ~FontAtlasImage();

    static Ref<FontAtlasImage> create(Sizei size, int32_t mip_count, Format format, Span<const uint8_t> data);
    static Ref<FontAtlasImage> create(Sizei size, int32_t mip_count, Format format);

    // getter
    inline Sizei size() const SKR_NOEXCEPT
    {
        return _size;
    }
    inline int32_t                width() const SKR_NOEXCEPT { return _size.width; }
    inline int32_t                height() const SKR_NOEXCEPT { return _size.height; }
    inline Format                 format() const SKR_NOEXCEPT { return _format; }
    inline int32_t                mip_count() const SKR_NOEXCEPT { return _mip_count; }
    inline const PackedByteArray& data() const SKR_NOEXCEPT { return _data; }
    inline ImageEntry*            entry() const SKR_NOEXCEPT { return _entry; }
    inline bool                   dirty() const SKR_NOEXCEPT { return _dirty; }

    // modify & flush
    inline void     set_size(Sizei size) SKR_NOEXCEPT { _size = size; }
    inline void     set_format(Format format) SKR_NOEXCEPT { _format = format; }
    inline void     set_mip_count(int32_t mip_count) SKR_NOEXCEPT { _mip_count = mip_count; }
    inline uint8_t* resize_for_write(size_t size) SKR_NOEXCEPT
    {
        mark_dirty();
        _data.resize(size);
        return _data.data();
    }
    inline PackedByteArray& data_for_write() SKR_NOEXCEPT
    {
        mark_dirty();
        return _data;
    }
    inline void set_data(Span<const uint8_t> data)
    {
        mark_dirty();
        _data.assign(data.begin(), data.end());
    }
    inline void mark_dirty() SKR_NOEXCEPT { _dirty = true; }
    inline void flush_update() SKR_NOEXCEPT
    {
        if (_dirty)
        {
            ImageEntry::UpdateDesc desc = {};
            desc.format = _format;
            desc.size = _size;
            desc.mip_count = 0; // TODO. mip maps
            desc.data = { _data.data(), _data.size() };
            _entry->update(desc);
            _dirty = false;
        }
    }
    inline Image* render_image() const SKR_NOEXCEPT
    {
        auto result = _entry->request_image_of_size({});
        return result->state() == ::skr::gui::EResourceState::Okay ? result.get() : nullptr;
    }

    // RID
    static RID_PtrOwner<FontAtlasImage>& texture_owner();
    inline RID                           get_rid() const SKR_NOEXCEPT { return _rid; }

private:
    Sizei           _size = {};
    Format          _format = Format::Unknown;
    int32_t         _mip_count = 0;
    PackedByteArray _data = {};
    ImageEntry*     _entry = nullptr;
    bool            _dirty = true;
    ImageProvider*  _provider = nullptr;
    RID             _rid = {};
};
} // namespace godot