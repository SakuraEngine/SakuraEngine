#pragma once
#include "../attributes.hxx"
#include "../types/ray.hxx"

enum struct RayQueryFlags : uint32
{
    None = 0x0,
    ForceOpaque = 0x1,
    ForceNonOpaque = 0x2,
    AcceptFirstAndEndSearch = 0x4,
    
    CullBackFace = 0x10,
    CullFrontFace = 0x20,

    CullOpaque = 0x40,
    CullNonOpaque = 0x80,

    CullTriangle = 0x100,
    CullProcedural = 0x200
};
inline constexpr RayQueryFlags operator|(RayQueryFlags lhs, RayQueryFlags rhs) {
    return static_cast<RayQueryFlags>(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
}

template <RayQueryFlags flags>
struct [[builtin("ray_query")]] RayQuery {
    RayQuery() = default;
    RayQuery(const RayQuery&) = delete;
    RayQuery(RayQuery&) = delete;
    RayQuery operator =(const RayQuery&) = delete;
    RayQuery operator =(RayQuery&) = delete;

    [[callop("RAY_QUERY_PROCEED")]] 
    bool Proceed();
    
    [[callop("RAY_QUERY_CANDIDATE_STATUS")]] 
    HitStatus CandidateStatus();
    
    [[callop("RAY_QUERY_CANDIDATE_TRIANGLE_BARYCENTRICS")]] 
    float2 CandidateTriangleBarycentrics();
    
    [[callop("RAY_QUERY_CANDIDATE_INSTANCE_INDEX")]] 
    uint CandidateInstanceIndex();

    [[callop("RAY_QUERY_CANDIDATE_GEOMETRY_INDEX")]] 
    uint CandidateGeometryIndex();

    [[callop("RAY_QUERY_CANDIDATE_PRIMIVE_INDEX")]] 
    uint CandidatePrimitiveIndex();

    [[callop("RAY_QUERY_CANDIDATE_PROCEDURAL_DISTANCE")]] 
    float CandidateProceduralDistance();
    
    [[callop("RAY_QUERY_CANDIDATE_TRIANGLE_RAY_T")]] 
    float CandidateTriangleRayT();
    

    [[callop("RAY_QUERY_COMMITTED_STATUS")]] 
    HitStatus CommittedStatus();
    
    [[callop("RAY_QUERY_COMMITTED_TRIANGLE_BARYCENTRICS")]] 
    float2 CommittedTriangleBarycentrics();
    
    [[callop("RAY_QUERY_COMMITTED_INSTANCE_INDEX")]] 
    uint CommittedInstanceIndex();
    
    [[callop("RAY_QUERY_COMMITTED_GEOMETRY_INDEX")]] 
    uint CommittedGeometryIndex();

    [[callop("RAY_QUERY_COMMITTED_PRIMIVE_INDEX")]] 
    uint CommittedPrimitiveIndex();
    
    [[callop("RAY_QUERY_COMMITTED_PROCEDURAL_DISTANCE")]] 
    float CommittedProceduralDistance();
    
    [[callop("RAY_QUERY_COMMITTED_RAY_T")]] 
    float CommittedRayT();


    [[callop("RAY_QUERY_WORLD_RAY_ORIGIN")]] 
    float3 WorldRayOrigin();
    
    [[callop("RAY_QUERY_WORLD_RAY_DIRECTION")]] 
    float3 WorldRayDirection();
    
    template <typename RayType>
    [[callop("RAY_QUERY_TRACE_RAY_INLINE")]] 
    void TraceRayInline(const RaytracingAccelerationStructure& AS, uint32 mask, const RayType& ray);

    [[callop("RAY_QUERY_COMMIT_TRIANGLE")]]
    void CommitTriangle();

    [[callop("RAY_QUERY_TERMINATE")]] 
    void Terminate();
};

using RayQueryAll = RayQuery<RayQueryFlags::None>;
using RayQueryAny = RayQuery<RayQueryFlags::AcceptFirstAndEndSearch>;