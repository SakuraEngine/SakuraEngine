#pragma once
#include "./../attributes.hxx"
#include "./../types/vec.hxx"

template <typename Type>
struct [[builtin("constant_buffer")]] ConstantBuffer : public Type
{
    
};

template<typename Type, uint32 cache_flags = BufferFlags::ReadOnly>
struct [[builtin("struct_buffer")]] SBuffer {
	using ElementType = Type;
	inline static constexpr auto flags = cache_flags;

	[[callop("BUFFER_READ")]] const Type& Load(uint32 loc) const;
	[[callop("BUFFER_WRITE")]] void Store(uint32 loc, const Type& value) requires((cache_flags & BufferFlags::ReadWrite) != 0);
	[[access]] const Type& operator[](uint32 loc) const;
	[[access]] Type& operator[](uint32 loc) requires((cache_flags & BufferFlags::ReadWrite) != 0);
};

template<typename Type, uint32 cache_flags = BufferFlags::ReadOnly>
struct [[builtin("texel_buffer")]] TBuffer {
	using ElementType = Type;
	inline static constexpr auto flags = cache_flags;

	[[callop("BUFFER_READ")]] const Type& Load(uint32 loc) const;
	[[callop("BUFFER_WRITE")]] void Store(uint32 loc, const Type& value) requires((cache_flags & BufferFlags::ReadWrite) != 0);
	[[access]] const Type& operator[](uint32 loc) const;
	[[access]] Type& operator[](uint32 loc) requires((cache_flags & BufferFlags::ReadWrite) != 0);
};

template<typename U, uint32 cache_flags>
struct [[builtin("byte_buffer")]] BBuffer {
	
	template<typename T = U>
	[[callop("BYTE_BUFFER_READ")]] T Load(uint32 byte_index) const;
	
	template<typename T = U>
	[[callop("BYTE_BUFFER_WRITE")]] void Store(uint32 byte_index, const T& val) requires((cache_flags & BufferFlags::ReadWrite) != 0);

	[[callop("BYTE_BUFFER_LOAD")]] uint Load(uint byte_index) const;
	[[callop("BYTE_BUFFER_LOAD2")]] uint2 Load2(uint byte_index) const;
	[[callop("BYTE_BUFFER_LOAD3")]] uint3 Load3(uint byte_index) const;
	[[callop("BYTE_BUFFER_LOAD4")]] uint4 Load4(uint byte_index) const;

	[[callop("BYTE_BUFFER_STORE")]] void Store(uint byte_index, uint value) requires((cache_flags & BufferFlags::ReadWrite) != 0);
	[[callop("BYTE_BUFFER_STORE2")]] void Store2(uint byte_index, uint2 value) requires((cache_flags & BufferFlags::ReadWrite) != 0);
	[[callop("BYTE_BUFFER_STORE3")]] void Store3(uint byte_index, uint3 value) requires((cache_flags & BufferFlags::ReadWrite) != 0);
	[[callop("BYTE_BUFFER_STORE4")]] void Store4(uint byte_index, uint4 value) requires((cache_flags & BufferFlags::ReadWrite) != 0);
};

/*
template<typename Type>
using Buffer = TBuffer<Type, BufferFlags::ReadOnly>;
template<typename Type>
using RWBuffer = TBuffer<Type, BufferFlags::ReadWrite>;
*/

template<typename Type>
using StructuredBuffer = SBuffer<Type, BufferFlags::ReadOnly>;
template<typename Type>
using RWStructuredBuffer = SBuffer<Type, BufferFlags::ReadWrite>;

template<typename Type>
using TexelBuffer = TBuffer<Type, BufferFlags::ReadOnly>;
template<typename Type>
using RWTexelBuffer = TBuffer<Type, BufferFlags::ReadWrite>;

using ByteAddressBuffer = BBuffer<uint32_t, BufferFlags::ReadOnly>;
using RWByteAddressBuffer = BBuffer<uint32_t, BufferFlags::ReadWrite>;