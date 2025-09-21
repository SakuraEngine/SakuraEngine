namespace skr::CppSL::MSL
{
const wchar_t* kMSLWaveIntrinsics = LR"__de___l___im__(

#define WaveGetLaneIndex() kCppSLBuiltins.WaveLaneIndex
#define WaveGetLaneCount() kCppSLBuiltins.WaveLaneCount
#define WaveIsFirstLane metal::simd_is_first
#define WaveGetMaxActiveIndex() metal::simd_max(WaveGetLaneIndex())
#define WaveActiveMax(X) metal::simd_max(X)
#define CountBallot _popcount_u4

// WaveActiveBallot for bool expression - returns bitmask of active lanes where predicate is true
uint4 WaveActiveBallot(bool expr) { 
    metal::simd_vote activeLaneMask = metal::simd_ballot(expr);
    // simd_ballot() returns a 64-bit integer-like object, but
    // SPIR-V callers expect a uint4. We must convert.
    // FIXME: This won't include higher bits if Apple ever supports
    // 128 lanes in an SIMD-group.
    return uint4(
        (uint)(((metal::simd_vote::vote_t)activeLaneMask)       & 0xFFFFFFFF),
#if defined(TARGET_IOS)
        0, // ios simd_vote is 32 bits
#else
        (uint)(((metal::simd_vote::vote_t)activeLaneMask >> 32) & 0xFFFFFFFF),
#endif
        uint2(0));
}

#define WaveReadLaneFirst metal::simd_broadcast_first
#define WaveActiveSum metal::simd_sum
#define WavePrefixSum metal::simd_prefix_exclusive_sum
#define QuadReadAcrossX(X) metal::quad_shuffle(X, (WaveGetLaneIndex() % 4u + 1) % 4)
#define QuadReadAcrossY(X) metal::quad_shuffle(X, (WaveGetLaneIndex() % 4u + 2) % 4)
#define WaveReadLaneAt(X, Y) metal::simd_shuffle(X, Y)
#define WaveActiveAnyTrue metal::simd_any
#define WaveActiveAllTrue metal::simd_all

inline uint _popcount_u4(uint4 values)
{
    const uint4 counts = popcount(values);
    return counts.x + counts.y + counts.z + counts.w;
}

#define WavePrefixCountBits(EXPR) _WavePrefixCountBits(EXPR, WaveGetLaneIndex())

inline uint _WavePrefixCountBits(uint4 b, uint lane_id)
{
    uint4 mask = uint4(metal::extract_bits(0xFFFFFFFF, 0, min(lane_id, 32u)), metal::extract_bits(0xFFFFFFFF, 0, (uint)max((int)lane_id - 32, 0)), uint2(0));
    return _popcount_u4(b & mask);
}

inline uint _WavePrefixCountBits(bool expr, uint lane_id)
{
    const uint4 ballot = WaveActiveBallot(expr);
    uint4 mask = uint4(
            metal::extract_bits(0xFFFFFFFF, 0, min(lane_id, 32u)),
            metal::extract_bits(0xFFFFFFFF, 0, (uint)max((int)lane_id - 32, 0)),
            uint2(0));
    return _popcount_u4(ballot & mask);
}

#define WaveActiveCountBits(EXPR) _popcount_u4(WaveActiveBallot(EXPR))

// HLSL Memory Barrier functions for Metal
#define AllMemoryBarrier() metal::threadgroup_barrier(metal::mem_flags::mem_threadgroup | metal::mem_flags::mem_device)
#define AllMemoryBarrierWithGroupSync() metal::threadgroup_barrier(metal::mem_flags::mem_threadgroup | metal::mem_flags::mem_device);
#define DeviceMemoryBarrier() metal::threadgroup_barrier(metal::mem_flags::mem_device);
#define DeviceMemoryBarrierWithGroupSync() metal::threadgroup_barrier(metal::mem_flags::mem_device);
#define GroupMemoryBarrier() metal::threadgroup_barrier(metal::mem_flags::mem_threadgroup);
#define GroupMemoryBarrierWithGroupSync() metal::threadgroup_barrier(metal::mem_flags::mem_threadgroup);

)__de___l___im__";

} // namespace CppSL::MSL