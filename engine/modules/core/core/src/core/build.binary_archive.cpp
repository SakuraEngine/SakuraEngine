#include <SkrCore/serialize/binary_archive.hpp>

// impl ArWriteBin
namespace skr
{
// ctor & dtor
ArWriteBin::ArWriteBin()
{
    _is_structured = false;

    _enable_string_enum = false;
    _enable_string_guid = false;
    _enable_string_md5 = false;
    _enable_string_sha256 = false;
}
ArWriteBin::~ArWriteBin()
{
}

// structured api
void ArWriteBin::impl_key(StringView key)
{
    error(u8"[ArWriteBin] does not support key()");
}
void ArWriteBin::impl_key_copy(StringView key)
{
    error(u8"[ArWriteBin] does not support key_copy()");
}
void ArWriteBin::impl_begin_array()
{
    error(u8"[ArWriteBin] does not support begin_array()");
}
void ArWriteBin::impl_end_array()
{
    error(u8"[ArWriteBin] does not support end_array()");
}
void ArWriteBin::impl_begin_object()
{
    error(u8"[ArWriteBin] does not support begin_object()");
}
void ArWriteBin::impl_end_object()
{
    error(u8"[ArWriteBin] does not support end_object()");
}

// typed data
void ArWriteBin::impl_primitive(EArchivePrimitiveType type, const void* data)
{
    impl_bytes(data, _primitive_size(type));
}
void ArWriteBin::impl_str(StringView str)
{
    // write length
    uint64_t length = str.size();
    impl_bytes(&length, sizeof(length));
    SKR_FAST_CHECK(checkpoint(), );

    // write content
    impl_bytes(str.data(), str.size());
}

// binary data support
void ArWriteBin::impl_bytes(const void* data, uint64_t size)
{
    _buffer.append(reinterpret_cast<const uint8_t*>(data), size);
}
void ArWriteBin::impl_bits(const void* data, uint64_t size, uint64_t offset)
{
    error(u8"[ArWriteBin] does not support bits()");
}

// reset
void ArWriteBin::reset()
{
    _error_tracker.reset();
}
void ArWriteBin::reset_buffer()
{
    _buffer.clear();
}
} // namespace skr

// impl ArReadBin
namespace skr
{
// ctor & dtor
ArReadBin::ArReadBin()
{
    _is_structured = false;

    _enable_string_enum = false;
    _enable_string_guid = false;
    _enable_string_md5 = false;
    _enable_string_sha256 = false;
}
ArReadBin::~ArReadBin() = default;

// structured api
void ArReadBin::impl_key(StringView key)
{
    error(u8"[ArReadBin] does not support key()");
}
bool ArReadBin::impl_has_value()
{
    error(u8"[ArReadBin] does not support has_value()");
    return true;
}
void ArReadBin::impl_begin_array()
{
    error(u8"[ArReadBin] does not support begin_array()");
}
uint64_t ArReadBin::impl_array_size()
{
    error(u8"[ArReadBin] does not support array_size()");
    return 0;
}
void ArReadBin::impl_end_array()
{
    error(u8"[ArReadBin] does not support end_array()");
}
void ArReadBin::impl_begin_object()
{
    error(u8"[ArReadBin] does not support begin_object()");
}
void ArReadBin::impl_end_object()
{
    error(u8"[ArReadBin] does not support end_object()");
}

// typed data
void ArReadBin::impl_primitive(EArchivePrimitiveType type, void* data)
{
    impl_bytes(data, _primitive_size(type));
}
StringView ArReadBin::impl_str_view()
{
    // read length
    uint64_t length = 0;
    impl_bytes(&length, sizeof(length));
    SKR_FAST_CHECK(checkpoint(), {});

    // read content
    uint64_t old_pos;
    if (_move_next(length, old_pos)) [[likely]]
    {
        return {
            reinterpret_cast<const skr_char8*>(_buffer.data() + old_pos),
            length
        };
    }
    else
    {
        return {};
    }
}

// binary data support
void ArReadBin::impl_bytes(void* data, uint64_t size)
{
    uint64_t old_pos;
    if (_move_next(size, old_pos)) [[likely]]
    {
        memcpy(data, _buffer.data() + old_pos, size);
    }
}
void ArReadBin::impl_bits(void* data, uint64_t size, uint64_t offset)
{
    error(u8"[ArReadBin] does not support bits()");
}

// step 1. prepare data
void ArReadBin::use_buffer(Span<const uint8_t> buffer)
{
    _buffer = buffer;
    reset();
}
Span<const uint8_t> ArReadBin::buffer() const { return _buffer; }

// step 2. reset read position
void ArReadBin::reset()
{
    _read_pos = 0;
    _error_tracker.reset();
}
void ArReadBin::reset_buffer()
{
    _buffer = {};
}

// helpers
bool ArReadBin::_move_next(uint64_t size, uint64_t& old_pos)
{
    if (_read_pos + size > _buffer.size()) [[unlikely]]
    {
        error(u8"[ArReadBin] out of range ({} + {} > {})", _read_pos, size, _buffer.size());
        return false;
    }
    else
    {
        old_pos = _read_pos;
        _read_pos += size;
        return true;
    }
}
} // namespace skr