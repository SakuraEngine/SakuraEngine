#include "SkrBase/misc/bit.hpp"
#include "SkrBase/misc/demangle.hpp"
#include "SkrCore/memory/memory.h"
#include "SkrContainers/sptr.hpp"
#include "SkrContainers/string.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrCore/log.h"
#include "SkrSerde/binary/reader.h"
#include <cmath>

// blob arena
skr_blob_arena_t::skr_blob_arena_t()
    : buffer(nullptr)
    , _base(0)
    , align(0)
    , offset(0)
    , capacity(0)
{
}

skr_blob_arena_t::skr_blob_arena_t(void* buffer, uint64_t base, uint32_t size, uint32_t align)
    : buffer(buffer)
    , _base(base)
    , align(align)
    , offset(size)
    , capacity(size)
{
}

skr_blob_arena_t::skr_blob_arena_t(skr_blob_arena_t&& other)
    : buffer(other.buffer)
    , _base(other._base)
    , align(other.align)
    , offset(other.offset)
    , capacity(other.capacity)
{
    other.buffer   = nullptr;
    other._base    = 0;
    other.offset   = 0;
    other.capacity = 0;
}

skr_blob_arena_t& skr_blob_arena_t::operator=(skr_blob_arena_t&& other)
{
    buffer         = other.buffer;
    _base          = other._base;
    align          = other.align;
    offset         = other.offset;
    capacity       = other.capacity;
    other.buffer   = nullptr;
    other._base    = 0;
    other.offset   = 0;
    other.capacity = 0;
    return *this;
}

skr_blob_arena_t::~skr_blob_arena_t()
{
#ifdef SKR_BLOB_ARENA_CHECK
    SKR_ASSERT(size == 0);
#endif
    if (buffer)
        sakura_free_aligned(buffer, align);
}

#ifdef SKR_BLOB_ARENA_CHECK
void skr_blob_arena_t::release(size_t size) { this->size -= size; }
#endif
skr_blob_arena_builder_t::skr_blob_arena_builder_t(size_t align)
    : buffer(nullptr)
    , bufferAlign(align)
    , offset(0)
    , capacity(0)
{
}

skr_blob_arena_builder_t::~skr_blob_arena_builder_t()
{
    if (buffer)
    {
        sakura_free_aligned(buffer, bufferAlign);
    }
}

skr_blob_arena_t skr_blob_arena_builder_t::build()
{
    skr_blob_arena_t arena(buffer, 0, (uint32_t)offset, (uint32_t)bufferAlign);
    buffer   = nullptr;
    offset   = 0;
    capacity = 0;
    return arena;
}

size_t skr_blob_arena_builder_t::allocate(size_t size, size_t align)
{
    void* ptr = (char*)buffer + offset;
    // alignup ptr
    ptr                = (void*)(((size_t)ptr + align - 1) & ~(align - 1));
    uint32_t retOffset = (uint32_t)((char*)ptr - (char*)buffer);
    if (retOffset + size > capacity)
    {
        size_t new_capacity = capacity * 2;
        if (new_capacity < retOffset + size)
            new_capacity = retOffset + size;
        SKR_ASSERT(align <= bufferAlign);
        void* new_buffer = sakura_malloc_aligned(new_capacity, bufferAlign);
        if (buffer)
        {
            memcpy(new_buffer, buffer, offset);
            sakura_free_aligned(buffer, bufferAlign);
        }
        buffer   = new_buffer;
        capacity = new_capacity;
    }
    offset = retOffset + size;
    return retOffset;
}

namespace skr::binary
{
// primitive types
bool ReadTrait<bool>::Read(SBinaryReader* reader, bool& value)
{
    uint32_t v;
    if (!ReadTrait<uint32_t>::Read(reader, v))
    {
        SKR_LOG_FATAL(u8"failed to read boolean value!");
        return false;
    }
    value = v != 0;
    return true;
}

bool ReadTrait<int8_t>::Read(SBinaryReader* reader, int8_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<int16_t>::Read(SBinaryReader* reader, int16_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<int32_t>::Read(SBinaryReader* reader, int32_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<int64_t>::Read(SBinaryReader* reader, int64_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<uint8_t>::Read(SBinaryReader* reader, uint8_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<uint16_t>::Read(SBinaryReader* reader, uint16_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<uint32_t>::Read(SBinaryReader* reader, uint32_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<uint64_t>::Read(SBinaryReader* reader, uint64_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<float>::Read(SBinaryReader* reader, float& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<double>::Read(SBinaryReader* reader, double& value)
{
    return reader->read(&value, sizeof(value));
}

// skr types
bool ReadTrait<skr_float2_t>::Read(SBinaryReader* reader, skr_float2_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<skr_float3_t>::Read(SBinaryReader* reader, skr_float3_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<skr_float4_t>::Read(SBinaryReader* reader, skr_float4_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<skr_float4x4_t>::Read(SBinaryReader* reader, skr_float4x4_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<skr_rotator_t>::Read(SBinaryReader* reader, skr_rotator_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<skr_quaternion_t>::Read(SBinaryReader* reader, skr_quaternion_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<skr_md5_t>::Read(SBinaryReader* reader, skr_md5_t& md5)
{
    return ReadBytes(reader, &md5, sizeof(md5));
}

bool ReadTrait<skr_guid_t>::Read(SBinaryReader* reader, skr_guid_t& guid)
{
    return ReadBytes(reader, &guid, sizeof(guid));
}

bool ReadBlob(SBinaryReader* reader, skr::BlobId& out_id)
{
    // TODO: blob 应该特别处理
    uint64_t size;
    if (!skr::binary::Read(reader, size))
    {
        SKR_LOG_FATAL(u8"failed to read blob size!");
        return false;
    }
    auto blob = skr::IBlob::Create(nullptr, size, false);
    if (blob == nullptr)
    {
        SKR_LOG_FATAL(u8"failed to create blob!");
        return false;
    }

    if (!ReadBytes(reader, blob->get_data(), blob->get_size()))
    {
        SKR_LOG_FATAL(u8"failed to read blob content!");
        return false;
    }

    out_id = blob;
    return true;
}

bool ReadTrait<skr::IBlob*>::Read(SBinaryReader* reader, skr::IBlob*& out_blob)
{
    skr::BlobId new_blob = nullptr;
    bool        success  = ReadBlob(reader, new_blob);
    if ((!success) || (new_blob == nullptr))
    {
        SKR_LOG_FATAL(u8"failed to create blob!");
        return false;
    }

    out_blob = new_blob.get();
    out_blob->add_refcount();

    return true;
}

bool ReadTrait<skr::BlobId>::Read(SBinaryReader* reader, skr::BlobId& out_blob)
{
    auto success = ReadBlob(reader, out_blob);
    if ((!success) || (out_blob == nullptr))
    {
        SKR_LOG_FATAL(u8"failed to create blob!");
        return false;
    }
    return true;
}

bool ReadTrait<skr_blob_arena_t>::Read(SBinaryReader* reader, skr_blob_arena_t& arena)
{
    uint32_t size;
    if (!ReadTrait<uint32_t>::Read(reader, size))
    {
        SKR_LOG_FATAL(u8"failed to read blob arena size!");
        return false;
    }
    if (size == 0)
    {
        arena = skr_blob_arena_t(nullptr, 0, 0, 0);
        return true;
    }
    uint32_t align;
    if (!ReadTrait<uint32_t>::Read(reader, align))
    {
        SKR_LOG_FATAL(u8"failed to read blob arena alignment!");
        return false;
    }
    else
    {
        // FIXME: fix 0 alignment during serialization
        SKR_ASSERT(align != 0);
        align        = (align == 0) ? 1u : align;
        void* buffer = sakura_malloc_aligned(size, align);
        arena        = skr_blob_arena_t(buffer, 0, size, align);
        return true;
    }
}

bool ReadTrait<skr::String>::Read(SBinaryReader* reader, skr::String& str)
{
    uint32_t size;
    if (!ReadTrait<uint32_t>::Read(reader, size))
    {
        SKR_LOG_FATAL(u8"failed to read string buffer size!");
        return false;
    }
    skr::InlineVector<char8_t, 64> temp;
    temp.resize_default(size);
    if (!ReadBytes(reader, (void*)temp.data(), temp.size()))
    {
        SKR_LOG_FATAL(u8"failed to read string buffer size!");
        return false;
    }
    str = skr::String(skr::StringView((const char8_t*)temp.data(), (int32_t)temp.size()));
    return true;
}

bool ReadTrait<skr::StringView>::Read(SBinaryReader* reader, skr_blob_arena_t& arena, skr::StringView& str)
{
    uint32_t size;
    uint32_t offset;
    if (!ReadTrait<uint32_t>::Read(reader, size))
        return false;
    if (size == 0)
    {
        str = skr::StringView();
        return true;
    }
    if (!ReadTrait<uint32_t>::Read(reader, offset))
    {
        SKR_LOG_FATAL(u8"failed to read string buffer size (inside arena)!");
        return false;
    }

    auto strbuf_start = (char8_t*)arena.get_buffer() + offset;
    if (!ReadBytes(reader, strbuf_start, size))
    {
        SKR_LOG_FATAL(u8"failed to read string buffer content (inside arena)!");
        return false;
    }

    auto ptr = const_cast<char8_t*>(strbuf_start + size);
    *ptr     = u8'\0';
    str      = skr::StringView(strbuf_start, (int32_t)size);
    return true;
}
} // namespace skr::binary