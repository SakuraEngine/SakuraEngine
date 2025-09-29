#include "std/type_traits/builtins.hxx"
#include <std/std.hxx>

RWTexture2DArray<float4> skybox;

struct RootConstants
{
    uint CubeRes;
};

[[push_constant]]
ConstantBuffer<RootConstants> push_constants;

float4 face_colors[6] = {
    float4(1.0f, 0.4f, 0.2f, 1.0f), // +X
    float4(0.0f, 1.0f, 0.0f, 1.0f), // -X
    float4(0.0f, 0.0f, 1.0f, 1.0f), // +Y
    float4(1.0f, 1.0f, 0.0f, 1.0f), // -Y
    float4(1.0f, 0.0f, 1.0f, 1.0f), // +Z
    float4(0.0f, 1.0f, 1.0f, 1.0f)  // -Z
};

[[numthreads(16, 16, 1)]]
void cs([[sv_thread_id]] uint3 thread_idx)
{
    const uint cubeFace = thread_idx.z;
    const uint2 pixelPos = thread_idx.xy;
    const uint cubeRes = push_constants.CubeRes;

    uint3 src = uint3(pixelPos.x, pixelPos.y, cubeFace);
    skybox[src] = face_colors[cubeFace];
}