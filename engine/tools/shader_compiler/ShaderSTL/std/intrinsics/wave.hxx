#pragma once
#include "./../attributes.hxx"
#include "./../type_traits.hxx"

template<typename T>
[[callop("QuadReadAcrossDiagonal")]] 
extern T QuadReadAcrossDiagonal(T v);

template<typename T>
[[callop("QuadReadLaneAt")]] 
extern T QuadReadLaneAt(T v, uint line_id);

template<typename T>
[[callop("QuadReadAcrossX")]] 
extern T QuadReadAcrossX(T v);

template<typename T>
[[callop("QuadReadAcrossY")]] 
extern T QuadReadAcrossY(T v);

template<concepts::arithmetic T>
[[callop("WaveActiveAllEqual")]] 
extern vec<bool, vec_dim_v<T>> WaveActiveAllEqual(T v);

template<concepts::int_family T>
[[callop("WaveActiveBitAnd")]] 
extern T WaveActiveBitAnd(T v);

template<concepts::int_family T>
[[callop("WaveActiveBitOr")]] 
extern T WaveActiveBitOr(T v);

template<concepts::int_family T>
[[callop("WaveActiveBitXor")]] 
extern T WaveActiveBitXor(T v);

[[callop("WaveActiveCountBits")]]
extern uint32 WaveActiveCountBits(bool val);

template<concepts::arithmetic T>
[[callop("WaveActiveMax")]] 
extern T WaveActiveMax(T v);

template<concepts::arithmetic T>
[[callop("WaveActiveMin")]] 
extern T WaveActiveMin(T v);

template<concepts::arithmetic T>
[[callop("WaveActiveProduct")]] 
extern T WaveActiveProduct(T v);

template<concepts::arithmetic T>
[[callop("WaveActiveSum")]] 
extern T WaveActiveSum(T v);

[[callop("WaveActiveAllTrue")]] 
extern bool WaveActiveAllTrue(bool val);

[[callop("WaveActiveAnyTrue")]] 
extern bool WaveActiveAnyTrue(bool val);

[[callop("WaveActiveBallot")]] 
extern uint4 WaveActiveBallot(bool val);

[[callop("WaveGetLaneCount")]] 
extern uint32 WaveGetLaneCount();

[[callop("WaveGetLaneIndex")]] 
extern uint32 WaveGetLaneIndex();

[[callop("WaveIsFirstLane")]] 
extern bool WaveIsFirstLane();

[[callop("WavePrefixCountBits")]] 
extern uint32 WavePrefixCountBits(bool val);

template<concepts::arithmetic T>
[[callop("WavePrefixProduct")]] 
extern T WavePrefixProduct(T v);

template<concepts::arithmetic T>
[[callop("WavePrefixSum")]] 
extern T WavePrefixSum(T v);

template<concepts::arithmetic T>
[[callop("WaveReadLaneFirst")]] 
extern T WaveReadLaneFirst(T v);

template<typename T>
[[callop("WaveReadLaneAt")]] 
extern T WaveReadLaneAt(uint32 lane_index);

[[callop("firstbithigh")]] 
extern int firstbithigh(int val);