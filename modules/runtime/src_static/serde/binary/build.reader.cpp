#include "resource/resource_handle.h"
#include "serde/binary/reader.h"
#include "platform/memory.h"
#include "misc/bits.hpp"
#include "misc/log.h"
#include <cmath>

#include <EASTL/fixed_string.h>

skr_blob_arena_t::skr_blob_arena_t()
    : buffer(nullptr), _base(0), align(0), offset(0), capacity(0)  {}
skr_blob_arena_t::skr_blob_arena_t(void* buffer, uint64_t base, uint32_t size, uint32_t align)
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
    {
        sakura_free_aligned(buffer, bufferAlign);
    }
}

skr_blob_arena_t skr_blob_arena_builder_t::build()
{
    skr_blob_arena_t arena(buffer, 0, (uint32_t)offset, (uint32_t)bufferAlign);
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
    {
        SKR_LOG_FATAL("failed to read boolean value! ret code: %d", ret);
        return ret;
    }
    value = v != 0;
    return ret;
}

template<class T>
int ReadBitpacked(skr_binary_reader_t* reader, T& value, IntegerPackConfig<T> config)
{
    SKR_ASSERT(config.min <= config.max);
    SKR_ASSERT(reader->vread_bits);
    if(!reader->vread_bits)
    {
        SKR_LOG_ERROR("vread_bits is not implemented. falling back to vread");
        return reader->read(&value, sizeof(T));
    }
    auto bits = 64 - skr::CountLeadingZeros64(config.max - config.min);
    T compressed = 0;
    int ret = reader->read_bits(&compressed, bits);
    if(ret != 0)
        return ret;
    value = compressed + config.min;
    return ret;
}

int ReadTrait<uint8_t>::Read(skr_binary_reader_t* reader, uint8_t& value, IntegerPackConfig<uint8_t> config)
{
    return ReadBitpacked(reader, value, config);
}

int ReadTrait<uint16_t>::Read(skr_binary_reader_t* reader, uint16_t& value, IntegerPackConfig<uint16_t> config)
{
    return ReadBitpacked(reader, value, config);
}

int ReadTrait<uint32_t>::Read(skr_binary_reader_t* reader, uint32_t& value, IntegerPackConfig<uint32_t> config)
{
    return ReadBitpacked(reader, value, config);
}

int ReadTrait<uint64_t>::Read(skr_binary_reader_t* reader, uint64_t& value, IntegerPackConfig<uint64_t> config)
{
    return ReadBitpacked(reader, value, config);
}

int ReadTrait<int32_t>::Read(skr_binary_reader_t* reader, int32_t& value, IntegerPackConfig<int32_t> config)
{
    return ReadBitpacked(reader, value, config);
}

int ReadTrait<int64_t>::Read(skr_binary_reader_t* reader, int64_t& value, IntegerPackConfig<int64_t> config)
{
    return ReadBitpacked(reader, value, config);
}

template<class T, class ScalarType>
int ReadBitpacked(skr_binary_reader_t* reader, T& value, VectorPackConfig<ScalarType> config)
{
    ScalarType* array = (ScalarType*)&value;
    static constexpr size_t size = sizeof(T) / sizeof(ScalarType);
	using IntType = std::conditional_t<sizeof(ScalarType) == 4, int32_t, int64_t>;

	uint8_t ComponentBitCountAndExtraInfo = 0;
    auto ret = reader->read(&ComponentBitCountAndExtraInfo, 1);
	const uint32_t ComponentBitCount = ComponentBitCountAndExtraInfo & 63U;
	const uint32_t ExtraInfo = ComponentBitCountAndExtraInfo >> 6U;

	if (ComponentBitCount > 0U)
	{
		int64_t values[size] {0};

        for(size_t i = 0; i < size; ++i)
        {
            ret = reader->read_bits(&values[i], ComponentBitCount);
            if(ret != 0)
            {
                auto type = skr::demangle<T>();
                SKR_LOG_FATAL("failed to read packed bits of type %s! ret code: %d", type.c_str(), ret);
                return ret;
            }
        }

		// Sign-extend the values. The most significant bit read indicates the sign.
		const int64_t SignBit = (1ULL << (ComponentBitCount - 1U));
        for(size_t i = 0; i < size; ++i)
        {
            values[i] = (values[i] ^ SignBit) - SignBit;
        }

		// Apply scaling if needed.
		if (ExtraInfo)
		{
            for (size_t i = 0; i < size; ++i)
            {
                array[i] = ScalarType(values[i]) / config.scale;
            }
		}
		else
		{
            for (size_t i = 0; i < size; ++i)
            {
                array[i] = ScalarType(values[i]);
            }
		}

		return 0;
	}
	else
	{
        ret = reader->read(array, sizeof(T));
        if(ret != 0)
            return ret;
        bool containsNan = false;
        for(int i = 0; i < size; ++i)
        {
            if(std::isnan(array[i]))
            {
                containsNan = true;
                break;
            }
        }
        if(containsNan)
        {
            SKR_LOG_ERROR("ReadBitpacked: Value isn't finite. Clearing for safety.");
            for(int i = 0; i < size; ++i)
            {
                array[i] = 0;
            }
        }
        return 0;
	}

	// Should not get here so something is very wrong.
	return -1;
}

int ReadTrait<skr_float2_t>::Read(skr_binary_reader_t* reader, skr_float2_t& value, VectorPackConfig<float> cfg)
{
    return ReadBitpacked(reader, value, cfg);
}

int ReadTrait<skr_float3_t>::Read(skr_binary_reader_t* reader, skr_float3_t& value, VectorPackConfig<float> cfg)
{
    return ReadBitpacked(reader, value, cfg);
}

int ReadTrait<skr_float4_t>::Read(skr_binary_reader_t* reader, skr_float4_t& value, VectorPackConfig<float> cfg)
{
    return ReadBitpacked(reader, value, cfg);
}

int ReadTrait<skr_rotator_t>::Read(skr_binary_reader_t* reader, skr_rotator_t& value, VectorPackConfig<float> cfg)
{
    return ReadBitpacked(reader, value, cfg);
}

int ReadTrait<skr_quaternion_t>::Read(skr_binary_reader_t* reader, skr_quaternion_t& value, VectorPackConfig<float> cfg)
{
    return ReadBitpacked(reader, value, cfg);
}

int ReadTrait<skr_float4x4_t>::Read(skr_binary_reader_t* reader, skr_float4x4_t& value, VectorPackConfig<float> cfg)
{
    return ReadBitpacked(reader, value, cfg);
}

int ReadTrait<skr::string>::Read(skr_binary_reader_t* reader, skr::string& str)
{
    uint32_t size;
    int ret = ReadTrait<uint32_t>::Read(reader, size);
    if (ret != 0)
    {
        SKR_LOG_FATAL("failed to read string buffer size! ret code: %d", ret);
        return ret;
    }
    eastl::fixed_string<char8_t, 64> temp;
    temp.resize(size);
    ret = ReadBytes(reader, (void*)temp.c_str(), temp.size());
    if (ret != 0)
    {
        SKR_LOG_FATAL("failed to read string buffer size! ret code: %d", ret);
        return ret;
    }

    str = skr::string(skr::string_view((const char8_t*)temp.c_str(), (int32_t)temp.size()));
    return ret;
}

int ReadTrait<skr::string_view>::Read(skr_binary_reader_t* reader, skr_blob_arena_t& arena, skr::string_view& str)
{
    uint32_t size;
    uint32_t offset;
    int ret = ReadTrait<uint32_t>::Read(reader, size);
    if (ret != 0)
        return ret;
    if(size == 0)
    {
        str = skr::string_view();
        return 0;
    }
    ret = ReadTrait<uint32_t>::Read(reader, offset);
    if (ret != 0)
    {
        SKR_LOG_FATAL("failed to read string buffer size (inside arena)! ret code: %d", ret);
        return ret;
    }

    auto strbuf_start = (char8_t*)arena.get_buffer() + offset;
    ret = ReadBytes(reader, strbuf_start, size);
    if (ret != 0)
    {
        SKR_LOG_FATAL("failed to read string buffer content (inside arena)! ret code: %d", ret);
        return ret;
    }

    auto ptr = const_cast<char8_t*>(strbuf_start + size);
    *ptr = u8'\0';
    str = skr::string_view(strbuf_start, (int32_t)size);
    return ret;
}

int ReadTrait<skr_md5_t>::Read(skr_binary_reader_t* reader, skr_md5_t& md5)
{
    return ReadBytes(reader, &md5, sizeof(md5));
}

int ReadTrait<skr_guid_t>::Read(skr_binary_reader_t* reader, skr_guid_t& guid)
{
    return ReadBytes(reader, &guid, sizeof(guid));
}

int ReadTrait<skr_resource_handle_t>::Read(skr_binary_reader_t* reader, skr_resource_handle_t& handle)
{
    skr_guid_t guid;
    int ret = ReadTrait<skr_guid_t>::Read(reader, guid);
    if (ret != 0)
    {
        SKR_LOG_FATAL("failed to read resource handle guid! ret code: %d", ret);
        return ret;
    }
    handle.set_guid(guid);
    return ret;
}

int ReadTrait<skr_blob_t>::Read(skr_binary_reader_t* reader, skr_blob_t& blob)
{
    // TODO: blob 应该特别处理
    skr_blob_t temp;
    int ret = skr::binary::Read(reader, temp.size);
    if (ret != 0)
    {
        SKR_LOG_FATAL("failed to read blob size! ret code: %d", ret);
        return ret;
    }
    temp.bytes = (uint8_t*)sakura_malloc(temp.size);
    ret = ReadBytes(reader, temp.bytes, temp.size);
    if (ret != 0)
    {
        SKR_LOG_FATAL("failed to read blob content! ret code: %d", ret);
        return ret;
    }
    blob = std::move(temp);
    return ret;
}

int ReadTrait<skr_blob_arena_t>::Read(skr_binary_reader_t* reader, skr_blob_arena_t& arena)
{
    uint32_t size;
    int ret = ReadTrait<uint32_t>::Read(reader, size);
    if (ret != 0)
    {
        SKR_LOG_FATAL("failed to read blob arena size! ret code: %d", ret);
        return ret;
    }
    if (size == 0)
    {
        arena = skr_blob_arena_t(nullptr, 0, 0, 0);
        return ret;
    }
    uint32_t align;
    ret = ReadTrait<uint32_t>::Read(reader, align);
    if (ret != 0)
    {
        SKR_LOG_FATAL("failed to read blob arena alignment! ret code: %d", ret);
        return ret;
    }
    else
    {
        // FIXME: fix 0 alignment during serialization
        SKR_ASSERT(align != 0);
        align = (align == 0) ? 1u : align;
        void* buffer = sakura_malloc_aligned(size, align);
        arena = skr_blob_arena_t(buffer, 0, size, align);
        return ret;
    }

}

} // namespace skr::binary