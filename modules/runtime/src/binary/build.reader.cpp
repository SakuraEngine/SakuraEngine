#include "binary/reader.h"
#include "platform/memory.h"

skr_blob_arena_t::skr_blob_arena_t()
: buffer(nullptr), align(0), offset(0), capacity(0)  {}
skr_blob_arena_t::skr_blob_arena_t(size_t size, size_t align)
 : align(align), offset(0), capacity(size)
{ buffer = sakura_malloc_aligned(size, align); }
skr_blob_arena_t::skr_blob_arena_t(void* buffer, size_t size, size_t align)
 : buffer(buffer), align(align), offset(size), capacity(size) {}
skr_blob_arena_t::skr_blob_arena_t(skr_blob_arena_t&& other)
    : buffer(other.buffer), align(other.align), offset(other.offset), capacity(other.capacity)
    { other.buffer = nullptr; other.offset = 0; other.capacity = 0; }
skr_blob_arena_t& skr_blob_arena_t::operator=(skr_blob_arena_t&& other)
{
    buffer = other.buffer;
    align = other.align;
    offset = other.offset;
    capacity = other.capacity;
    other.buffer = nullptr;
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
void* skr_blob_arena_t::allocate(size_t size, size_t align)
{
    SKR_ASSERT(size <= capacity);
    SKR_ASSERT(align <= this->align);
    SKR_ASSERT(offset + size <= capacity);
    void* ptr = (char*)buffer + offset;
    // alignup ptr
    ptr = (void*)(((size_t)ptr + align - 1) & ~(align - 1));
    offset = (char*)ptr - (char*)buffer + size;
    return ptr;
}

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
    skr_blob_arena_t arena(buffer, offset, bufferAlign);
    buffer = nullptr;
    offset = 0;
    capacity = 0;
    return arena;
}

void* skr_blob_arena_builder_t::allocate(size_t size, size_t align)
{
    if(offset + size > capacity)
    {
        size_t new_capacity = capacity * 2;
        if(new_capacity < offset + size)
            new_capacity = offset + size;
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
    void* ptr = (char*)buffer + offset;
    // alignup ptr
    ptr = (void*)(((size_t)ptr + align - 1) & ~(align - 1));
    offset = (char*)ptr - (char*)buffer + size;
    return ptr;
}

namespace skr::binary
{
int ReadHelper<bool>::Read(skr_binary_reader_t* reader, bool& value)
{
    uint32_t v;
    int ret = ReadHelper<uint32_t>::Read(reader, v);
    if (ret != 0)
        return ret;
    value = v != 0;
    return ret;
}

int ReadHelper<skr::string>::Read(skr_binary_reader_t* reader, skr::string& str)
{
    uint32_t size;
    int ret = ReadHelper<uint32_t>::Read(reader, size);
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

int ReadHelper<skr::string_view>::Read(skr_binary_reader_t* reader, skr_blob_arena_t& arena, skr::string_view& str)
{
    uint32_t offset;
    int ret = ReadHelper<uint32_t>::Read(reader, offset);
    if (ret != 0)
        return ret;
    uint32_t size;
    ret = ReadHelper<uint32_t>::Read(reader, size);
    if (ret != 0)
        return ret;
    str = skr::string_view((const char*)arena.get_buffer() + offset, size);
    return ret;
}

int ReadHelper<skr_md5_t>::Read(skr_binary_reader_t* reader, skr_md5_t& md5)
{
    return ReadValue(reader, &md5, sizeof(md5));
}

int ReadHelper<skr_guid_t>::Read(skr_binary_reader_t* reader, skr_guid_t& guid)
{
    return ReadValue(reader, &guid, sizeof(guid));
}

int ReadHelper<skr_resource_handle_t>::Read(skr_binary_reader_t* reader, skr_resource_handle_t& handle)
{
    skr_guid_t guid;
    int ret = ReadHelper<skr_guid_t>::Read(reader, guid);
    if (ret != 0)
        return ret;
    handle.set_guid(guid);
    return ret;
}

int ReadHelper<skr_blob_t>::Read(skr_binary_reader_t* reader, skr_blob_t& blob)
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

int ReadHelper<skr_blob_arena_t>::Read(skr_binary_reader_t* reader, skr_blob_arena_t& arena)
{
    uint32_t size;
    int ret = ReadHelper<uint32_t>::Read(reader, size);
    if (ret != 0)
        return ret;
    uint32_t align;
    ret = ReadHelper<uint32_t>::Read(reader, align);
    if (ret != 0)
        return ret;
    void* buffer = sakura_malloc_aligned(size, align);
    ret = ReadValue(reader, buffer, size);
    if (ret != 0)
        return ret;
    arena = skr_blob_arena_t(buffer, size, align);
    return ret;
}

} // namespace skr::binary