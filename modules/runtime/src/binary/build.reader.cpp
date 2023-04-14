#include "resource/resource_handle.h"
#include "binary/reader.h"
#include "platform/memory.h"

skr_blob_arena_t::skr_blob_arena_t()
: buffer(nullptr), _base(0), align(0), offset(0), capacity(0)  {}
skr_blob_arena_t::skr_blob_arena_t(void* buffer, uint64_t base, size_t size, size_t align)
 : buffer(buffer), _base(base), align(align), offset(size), capacity(size) {}
skr_blob_arena_t::skr_blob_arena_t(skr_blob_arena_t&& other)
    : buffer(other.buffer), _base(other._base), align(other.align), offset(other.offset), capacity(other.capacity)
    { other.buffer = nullptr; other._base = 0; other.offset = 0; other.capacity = 0; }
skr_blob_arena_t& skr_blob_arena_t::operator=(skr_blob_arena_t&& other)
{
    buffer = other.buffer;
    _base = other._base;
    align = other.align;
    offset = other.offset;
    capacity = other.capacity;
    other.buffer = nullptr;
    other._base = 0;
    other.offset = 0;
    other.capacity = 0;
    return *this;
}
skr_blob_arena_t::~skr_blob_arena_t() 
{ 
#ifdef SKR_BLOB_ARENA_CHECK
    SKR_ASSERT(size == 0);
#endif
    if(buffer)
        sakura_free_aligned(buffer, align); 
}
#ifdef SKR_BLOB_ARENA_CHECK
void skr_blob_arena_t::release(size_t size) { this->size -= size; }
#endif

skr_blob_arena_builder_t::skr_blob_arena_builder_t(size_t align)
: buffer(nullptr), bufferAlign(align), offset(0), capacity(0)
{  }

skr_blob_arena_builder_t::~skr_blob_arena_builder_t()
{
    if(buffer)
        sakura_free_aligned(buffer, bufferAlign);
}

skr_blob_arena_t skr_blob_arena_builder_t::build()
{
    skr_blob_arena_t arena(buffer, 0, offset, bufferAlign);
    buffer = nullptr;
    offset = 0;
    capacity = 0;
    return arena;
}

size_t skr_blob_arena_builder_t::allocate(size_t size, size_t align)
{
    void* ptr = (char*)buffer + offset;
    // alignup ptr
    ptr = (void*)(((size_t)ptr + align - 1) & ~(align - 1));
    uint32_t retOffset = (uint32_t)((char*)ptr - (char*)buffer);
    if(retOffset + size > capacity)
    {
        size_t new_capacity = capacity * 2;
        if(new_capacity < retOffset + size)
            new_capacity = retOffset + size;
        SKR_ASSERT(align <= bufferAlign);
        void* new_buffer = sakura_malloc_aligned(new_capacity, bufferAlign);
        if(buffer)
        {
            memcpy(new_buffer, buffer, offset);
            sakura_free_aligned(buffer, bufferAlign);
        }
        buffer = new_buffer;
        capacity = new_capacity;
    }
    offset = retOffset + size;
    return retOffset;
}

namespace skr::binary
{
int ReadTrait<bool>::Read(skr_binary_reader_t* reader, bool& value)
{
    uint32_t v;
    int ret = ReadTrait<uint32_t>::Read(reader, v);
    if (ret != 0)
        return ret;
    value = v != 0;
    return ret;
}

int ReadTrait<skr::string>::Read(skr_binary_reader_t* reader, skr::string& str)
{
    uint32_t size;
    int ret = ReadTrait<uint32_t>::Read(reader, size);
    if (ret != 0)
        return ret;
    skr::string temp;
    temp.resize(size);
    ret = ReadValue(reader, (void*)temp.data(), temp.size());
    if (ret != 0)
        return ret;
    str = std::move(temp);
    return ret;
}

int ReadTrait<skr::string_view>::Read(skr_binary_reader_t* reader, skr_blob_arena_t& arena, skr::string_view& str)
{
    uint32_t offset;
    int ret = ReadTrait<uint32_t>::Read(reader, offset);
    if (ret != 0)
        return ret;
    uint32_t size;
    ret = ReadTrait<uint32_t>::Read(reader, size);
    if (ret != 0)
        return ret;
    str = skr::string_view((const char*)arena.get_buffer() + offset, size);
    return ret;
}

int ReadTrait<skr_md5_t>::Read(skr_binary_reader_t* reader, skr_md5_t& md5)
{
    return ReadValue(reader, &md5, sizeof(md5));
}

int ReadTrait<skr_guid_t>::Read(skr_binary_reader_t* reader, skr_guid_t& guid)
{
    return ReadValue(reader, &guid, sizeof(guid));
}

int ReadTrait<skr_resource_handle_t>::Read(skr_binary_reader_t* reader, skr_resource_handle_t& handle)
{
    skr_guid_t guid;
    int ret = ReadTrait<skr_guid_t>::Read(reader, guid);
    if (ret != 0)
        return ret;
    handle.set_guid(guid);
    return ret;
}

int ReadTrait<skr_blob_t>::Read(skr_binary_reader_t* reader, skr_blob_t& blob)
{
    // TODO: blob 应该特别处理
    skr_blob_t temp;
    int ret = skr::binary::Read(reader, temp.size);
    if (ret != 0)
        return ret;
    temp.bytes = (uint8_t*)sakura_malloc(temp.size);
    ret = ReadValue(reader, temp.bytes, temp.size);
    if (ret != 0)
        return ret;
    blob = std::move(temp);
    return ret;
}

int ReadTrait<skr_blob_arena_t>::Read(skr_binary_reader_t* reader, skr_blob_arena_t& arena)
{
    uint64_t base;
    int ret = ReadTrait<uint64_t>::Read(reader, base);
    if (ret != 0)
        return ret;

    uint32_t size;
    ret = ReadTrait<uint32_t>::Read(reader, size);
    if (ret != 0)
        return ret;

    uint32_t align;
    ret = ReadTrait<uint32_t>::Read(reader, align);
    if (ret != 0)
        return ret;

    if (size == 0)
    {
        arena = skr_blob_arena_t(nullptr, base, 0, align);
        return ret;
    }
    else
    {
        // FIXME: fix 0 alignment during serialization
        SKR_ASSERT(align != 0);
        align = (align == 0) ? 1u : align;
        void* buffer = sakura_malloc_aligned(size, align);
        ret = ReadValue(reader, buffer, size);
        if (ret != 0)
            return ret;
        arena = skr_blob_arena_t(buffer, base, size, align);
        return ret;
    }

}

} // namespace skr::binary