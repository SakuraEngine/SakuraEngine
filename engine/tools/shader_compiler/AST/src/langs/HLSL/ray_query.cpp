namespace skr::CppSL::HLSL
{
const wchar_t* kHLSLRayIntrinsics = LR"__de___l___im__(
using AccelerationStructure = RaytracingAccelerationStructure;

RayDesc create_ray(float3 origin, float3 dir, float tmin, float tmax) { RayDesc r; r.Origin = origin; r.Direction = dir; r.TMin = tmin; r.TMax = tmax; return r; }
#define ray_query_proceed(q) (q).Proceed()

#define ray_query_committed_status(q) (q).CommittedStatus()
#define ray_query_committed_triangle_bary(q) (q).CommittedTriangleBarycentrics()
#define ray_query_committed_geometry_index(q) (q).CommittedGeometryIndex()
#define ray_query_committed_primitive_index(q) (q).CommittedPrimitiveIndex()
#define ray_query_committed_instance_index(q) (q).CommittedInstanceIndex()
#define ray_query_committed_procedual_distance(q) (q).CommittedRayProcedualDistance()
#define ray_query_committed_ray_t(q) (q).CommittedRayT()

#define ray_query_candidate_status(q) ((q).CandidateType() + 1)
#define ray_query_candidate_triangle_bary(q) (q).CandidateTriangleBarycentrics()
#define ray_query_candidate_geometry_index(q) (q).CandidateGeometryIndex()
#define ray_query_candidate_primitive_index(q) (q).CandidatePrimitiveIndex()
#define ray_query_candidate_instance_index(q) (q).CandidateInstanceIndex()
#define ray_query_candidate_procedual_distance(q) (q).CandidateRayProcedualDistance()
#define ray_query_candidate_triangle_ray_t(q) (q).CandidateTriangleRayT()

#define ray_query_world_ray_origin(q) (q).WorldRayOrigin()
#define ray_query_world_ray_direction(q) (q).WorldRayDirection()

#define ray_query_commit_triangle(q) (q).CommitNonOpaqueTriangleHit()
#define ray_query_terminate(q) (q).Terminate()

#define ray_query_trace_ray_inline(q, as, mask, ray) (q).TraceRayInline((as), RAY_FLAG_NONE, (mask), create_ray((ray).origin(), (ray).dir(), (ray).tmin(), (ray).tmax()))
#define ray_pipe_trace_ray(as, flags, mask, contri, multiplier, miss_index, ray, payload) TraceRay((as), (flags), (mask), (contri), (multiplier), (miss_index), create_ray((ray).origin(), (ray).dir(), (ray).tmin(), (ray).tmax()), payload)
)__de___l___im__";

} // namespace CppSL::HLSL