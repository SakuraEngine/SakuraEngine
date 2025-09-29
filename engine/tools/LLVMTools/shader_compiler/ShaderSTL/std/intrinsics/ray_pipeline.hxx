#pragma once
#include "./../attributes.hxx"
#include "./../types/vec.hxx"

struct RaytracingAccelerationStructure;
enum struct RayQueryFlags : uint32_t;

struct RayPipeline
{
    [[callop("RayPipeline::DispatchRaysIndex")]]
    static uint3 DispatchRaysIndex();

    template <typename RayDesc, typename PayloadT>
    [[callop("RayPipeline::TraceRay")]]
    static void TraceRay(const RaytracingAccelerationStructure& AccelerationStructure, const RayQueryFlags& RayFlags, uint InstanceInclusionMask, uint RayContributionToHitGroupIndex, uint MultiplierForGeometryContributionToHitGroupIndex, uint MissShaderIndex, RayDesc Ray, PayloadT& Payload);

    [[callop("RayPipeline::InstanceIndex")]]
    static uint InstanceIndex();

    [[callop("RayPipeline::GeometryIndex")]]
    static uint GeometryIndex();

    [[callop("RayPipeline::PrimitiveIndex")]]
    static uint PrimitiveIndex();

    [[callop("RayPipeline::RayTMin")]]
    static float RayTMin();

    [[callop("RayPipeline::RayTCurrent")]]
    static float RayTCurrent();

    [[callop("RayPipeline::WorldRayOrigin")]]
    static float3 WorldRayOrigin();

    [[callop("RayPipeline::WorldRayDirection")]]
    static float3 WorldRayDirection();
};