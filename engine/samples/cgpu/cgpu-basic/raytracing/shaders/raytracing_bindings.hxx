#pragma once
#include "std/std.hxx"

[[binding(0, 0)]]
RWStructuredBuffer<float4> OutputColor;

[[binding(1, 0)]]
RaytracingAccelerationStructure AS;