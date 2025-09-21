namespace skr::CppSL::MSL
{
const wchar_t* kMSLRayIntrinsics = LR"__de___l___im__(
// Ray tracing support
struct AccelerationStructure { metal::raytracing::instance_acceleration_structure as; };
using QueryFlags = uint32_t;

template <QueryFlags f>
struct RayQuery {
    thread metal::raytracing::intersection_query<metal::raytracing::triangle_data, metal::raytracing::instancing>* query;
    metal::raytracing::ray current_ray;
    bool initialized = false;
};

// Ray query macros
// Ray query initialization - creates intersection_query on the stack
#define ray_query_trace_ray_inline(q, _as, mask, r) \
    float3 _ray_origin_##__LINE__ = (r).origin(); \
    float3 _ray_direction_##__LINE__ = (r).dir(); \
    (q).current_ray = metal::raytracing::ray(_ray_origin_##__LINE__, _ray_direction_##__LINE__, (r).tmin(), (r).tmax()); \
    metal::raytracing::intersection_query<metal::raytracing::triangle_data, metal::raytracing::instancing> _query_##__LINE__((q).current_ray, (_as).as, mask); \
    (q).query = &_query_##__LINE__; \
    (q).initialized = true

#define ray_query_proceed(q) (q).query->next()

#define ray_query_committed_status(q) ((q).query->get_committed_intersection_type() == metal::raytracing::intersection_type::triangle ? HitStatus__HitTriangle : HitStatus__Miss)
#define ray_query_committed_triangle_bary(q) float2((q).query->get_committed_triangle_barycentric_coord().x, (q).query->get_committed_triangle_barycentric_coord().y)
#define ray_query_committed_primitive_index(q) ((q).query->get_committed_primitive_id())
#define ray_query_committed_geometry_index(q) ((q).query->get_committed_geometry_id())
#define ray_query_committed_instance_index(q) ((q).query->get_committed_instance_id())
#define ray_query_committed_ray_t(q) ((q).query->get_committed_distance())

#define ray_query_candidate_status(q) ((q).query->get_candidate_intersection_type() == metal::raytracing::intersection_type::triangle ? HitStatus__HitTriangle : HitStatus__Miss)
#define ray_query_candidate_triangle_bary(q) float2((q).query->get_candidate_triangle_barycentric_coord().x, (q).query->get_candidate_triangle_barycentric_coord().y)
#define ray_query_candidate_primitive_index(q) ((q).query->get_candidate_primitive_id())
#define ray_query_candidate_geometry_index(q) ((q).query->get_candidate_geometry_id())
#define ray_query_candidate_instance_index(q) ((q).query->get_candidate_instance_id())
#define ray_query_candidate_triangle_ray_t(q) ((q).query->get_candidate_triangle_distance())

#define ray_query_world_ray_origin(q) float3((q).current_ray.origin)
#define ray_query_world_ray_direction(q) float3((q).current_ray.direction)

#define ray_query_commit_triangle(q) (q).query->commit_triangle_intersection()
#define ray_query_terminate(q) (q).query->abort()

)__de___l___im__";

} // namespace CppSL::MSL