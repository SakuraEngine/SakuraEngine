#pragma once
#include "vec.hxx"
#include "array.hxx"

struct Ray
{
public:
    Ray() = default;
    Ray(const float3& origin, const float3& dir, float t_min = 0.0f, float t_max = 1e30f)
        : _origin{ origin.x, origin.y, origin.z }
        , t_min(t_min)
        , _dir{ dir.x, dir.y, dir.z }
        , t_max(t_max)
    {
    }
    [[nodiscard, noignore]] float3 origin() const { return _origin; }
    [[nodiscard, noignore]] float3 dir() const { return _dir; }
    [[nodiscard, noignore]] float tmin() const { return t_min; }
    [[nodiscard, noignore]] float tmax() const { return t_max; }

    void set_origin(float3 o) { _origin = o; }
    void set_dir(float3 d) { _dir = d; }
    void set_tmin(float t) { t_min = t; }
    void set_tmax(float t) { t_max = t; }

private:
    float3 _origin;
    float t_min = 0.0f;
    float3 _dir;
    float t_max = 1e30f;
};

enum struct HitStatus : uint32
{
    Miss = 0,
    HitTriangle = 1,
    HitProcedural = 2
};

constexpr uint32 invalid_handle = uint32(-1);

struct CommittedHit
{
    const uint32 inst = invalid_handle;
    const uint32 prim = invalid_handle;
    const float2 bary;
    const HitStatus hit_type = HitStatus::Miss;
    float ray_t;
    [[nodiscard, noignore]] bool miss() const
    {
        return hit_type == HitStatus::Miss;
    }
    [[nodiscard, noignore]] bool hit_triangle() const
    {
        return hit_type == HitStatus::HitTriangle;
    }
    [[nodiscard, noignore]] bool hit_procedural() const
    {
        return hit_type == HitStatus::HitProcedural;
    }
    template <concepts::float_family T>
    T interpolate(const T& a, const T& b, const T& c)
    {
        return T(1.0f - bary.x - bary.y) * a + T(bary.x) * b + T(bary.y) * c;
    }
};

struct TriangleHit
{
public:
    TriangleHit() = default;
    TriangleHit(uint32 inst, uint32 prim, uint32 geom, float2 bary, float ray_t)
        : inst(inst)
        , prim(prim)
        , geom(geom)
        , bary(bary)
        , ray_t(ray_t)
    {
    }

    uint32 inst = invalid_handle;
    uint32 prim = invalid_handle;
    uint32 geom = invalid_handle;
    float2 bary = float2();
    float ray_t = 0.f;

    [[nodiscard, noignore]] bool miss() const
    {
        return inst == invalid_handle;
    }
    [[nodiscard, noignore]] bool hitted() const
    {
        return inst != invalid_handle;
    }
    template <concepts::float_family T>
    T interpolate(const T& a, const T& b, const T& c)
    {
        return T(1.0f - bary.x - bary.y) * a + T(bary.x) * b + T(bary.y) * c;
    }
};
template <concepts::float_family T>
T interpolate(float2 bary, const T& a, const T& b, const T& c)
{
    return T(1.0f - bary.x - bary.y) * a + T(bary.x) * b + T(bary.y) * c;
}

struct ProceduralHit
{
    uint32 inst;
    uint32 prim;
};