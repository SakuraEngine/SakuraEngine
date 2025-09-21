#pragma once
#include "./../resources/buffer.hxx"
#include "./../intrinsics/math.hxx"
#include "./dispatch.hxx"

template<typename T>
	requires(cppsl::is_same_v<T, float> || cppsl::is_same_v<float, element_of<T>>)
static typename copy_dim<uint, T>::type float_pack_to_uint(T val) {
	using RetType = typename copy_dim<uint, T>::type;
	RetType uvalue = bit_cast<RetType>(val);
	return ite(uvalue >> RetType(31u) == RetType(0u), uvalue | RetType(1u << 31u), ~uvalue);
}

template<typename T>
	requires(cppsl::is_same_v<T, uint> || cppsl::is_same_v<uint, element_of<T>>)
static typename copy_dim<float, T>::type uint_unpack_to_float(T val) {
	using RetType = typename copy_dim<uint, T>::type;
	RetType uvalue = ite(val >> RetType(31u) == 0, ~val, val & RetType(~(1u << 31u)));
	return bit_cast<typename copy_dim<float, T>::type>(uvalue);
}

static float float_atomic_min(RWStructuredBuffer<uint>& buffer, uint index, float value) {
	uint prev;
	InterlockedMin(buffer[index], float_pack_to_uint(value), prev);
	return uint_unpack_to_float(prev);
}
static float float_atomic_max(RWStructuredBuffer<uint>& buffer, uint index, float value) {
	uint prev;
	InterlockedMax(buffer[index], float_pack_to_uint(value), prev);
	return uint_unpack_to_float(prev);
}
static float float_atomic_add(
	RWStructuredBuffer<uint>& buffer,
	uint index,
	float value) {
	uint old = buffer.Load(index);
	while (true) {
		uint prev;
		InterlockedCompareExchange(buffer[index], old, bit_cast<uint>(bit_cast<float>(old) + value), prev);
		if (prev == old) {
			break;
		}
		old = prev;
	}
	return bit_cast<float>(old);
}