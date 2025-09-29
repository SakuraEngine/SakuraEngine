#pragma once
#include "./../attributes.hxx"
#include "./../types/vec.hxx"

[[callop("AllMemoryBarrier")]] 
extern void AllMemoryBarrier();

[[callop("AllMemoryBarrierWithGroupSync")]] 
extern void AllMemoryBarrierWithGroupSync();

[[callop("DeviceMemoryBarrier")]] 
extern void DeviceMemoryBarrier();

[[callop("DeviceMemoryBarrierWithGroupSync")]] 
extern void DeviceMemoryBarrierWithGroupSync();

[[callop("GroupMemoryBarrier")]] 
extern void GroupMemoryBarrier();

[[callop("GroupMemoryBarrierWithGroupSync")]] 
extern void GroupMemoryBarrierWithGroupSync();

template <concepts::int_family V, concepts::int_family T>
[[callop("InterlockedAdd")]]
extern void InterlockedAdd(V& v, T value, T& prev);

template <concepts::int_family V, concepts::int_family T>
[[callop("InterlockedAnd")]]
extern void InterlockedAnd(V& v, T value, T& prev);

template <concepts::arithmetic_scalar V, concepts::arithmetic_scalar T>
[[callop("InterlockedCompareExchange")]]
extern void InterlockedCompareExchange(V& v, T compare, T value, T& prev);

template <concepts::arithmetic_scalar V, concepts::arithmetic_scalar T>
[[callop("InterlockedCompareStore")]]
extern void InterlockedCompareStore(V& v, T compare, T value);

template <concepts::arithmetic_scalar V, concepts::arithmetic_scalar T>
[[callop("InterlockedExchange")]]
extern void InterlockedExchange(V& v, T value, T& prev);

template <concepts::int_family V, concepts::int_family T>
[[callop("InterlockedMax")]]
extern void InterlockedMax(V& v, T value, T& prev);

template <concepts::int_family V, concepts::int_family T>
[[callop("InterlockedMin")]]
extern void InterlockedMin(V& v, T value, T& prev);

template <concepts::int_family V, concepts::int_family T>
[[callop("InterlockedOr")]]
extern void InterlockedOr(V& v, T value, T& prev);

template <concepts::int_family V, concepts::int_family T>
[[callop("InterlockedXor")]]
extern void InterlockedXor(V& v, T value, T& prev);

// raster
[[callop("RASTER_DISCARD")]] 
extern void discard();

// try flatten next if-stmt
[[callop("FLATTEN")]] 
extern void flatten();	

// try un-flatten next if-stmt
[[callop("BRANCH")]] 
extern void branch();		  

[[callop("FORCE_CASE")]] 
extern void force_case();