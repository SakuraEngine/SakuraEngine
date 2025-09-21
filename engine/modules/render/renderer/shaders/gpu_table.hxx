#pragma once
#include "SkrRenderer/shared/gpu_scene.hpp" // IWYU pragma: export

template <typename T, uint32 cache_flags = BufferFlags::ReadOnly>
struct TypedAccessor;

template <typename T, uint32 cache_flags = BufferFlags::ReadOnly>
struct [[builtin("byte_buffer")]] GPUTable
{
public:
	const T& operator[](uint32 instance_index) const
    {
        skr::gpu::Row<T> accesser(instance_index);
        return accesser.Load(*this);
    }
	TypedAccessor<T, cache_flags> operator[](uint32 loc);

private:
    template <typename X>
    friend struct skr::gpu::Row;
    template <typename X>
    friend struct skr::gpu::Range;
    template <typename X, uint32_t N>
    friend struct skr::gpu::FixedRange;
    
    template<typename X>
	[[callop("BYTE_BUFFER_READ")]] X Load(uint32 byte_index) const;
	template<typename X>
	[[callop("BYTE_BUFFER_WRITE")]] void Store(uint32 byte_index, const X& val) requires((cache_flags & BufferFlags::ReadWrite) != 0);
	[[callop("BYTE_BUFFER_LOAD")]] uint Load(uint byte_index) const;
	[[callop("BYTE_BUFFER_LOAD2")]] uint2 Load2(uint byte_index) const;
	[[callop("BYTE_BUFFER_LOAD3")]] uint3 Load3(uint byte_index) const;
	[[callop("BYTE_BUFFER_LOAD4")]] uint4 Load4(uint byte_index) const;
	[[callop("BYTE_BUFFER_STORE")]] void Store(uint byte_index, uint value) requires((cache_flags & BufferFlags::ReadWrite) != 0);
	[[callop("BYTE_BUFFER_STORE2")]] void Store2(uint byte_index, uint2 value) requires((cache_flags & BufferFlags::ReadWrite) != 0);
	[[callop("BYTE_BUFFER_STORE3")]] void Store3(uint byte_index, uint3 value) requires((cache_flags & BufferFlags::ReadWrite) != 0);
	[[callop("BYTE_BUFFER_STORE4")]] void Store4(uint byte_index, uint4 value) requires((cache_flags & BufferFlags::ReadWrite) != 0);
};

template <typename T, uint32_t flags>
inline constexpr bool is_bbuffer_v<GPUTable<T, flags>> = true;

template <typename T, uint32 cache_flags>
struct TypedAccessor
{
    operator T() const
    {
        skr::gpu::Row<T> accesser(instance_index);
        return accesser.Load(buffer);
    }
    void operator=(const T& v) requires((cache_flags & BufferFlags::ReadWrite) != 0)
    {
        skr::gpu::Row<T> accesser(instance_index);
        accesser.Store(buffer, v);
    }
private:
    friend struct GPUTable<T, cache_flags>;
    TypedAccessor(GPUTable<T, cache_flags> buffer, uint32_t instance_index)
        : buffer(buffer), instance_index(instance_index)
    {

    }
    GPUTable<T, cache_flags> buffer;
    uint32_t instance_index;
};

template <typename T, uint32 cache_flags>
TypedAccessor<T, cache_flags> GPUTable<T, cache_flags>::operator[](uint32 loc)
{
    return TypedAccessor<T, cache_flags>(*this, loc);
}

template <typename T>
using RWGPUTable = GPUTable<T, BufferFlags::ReadWrite>; 
