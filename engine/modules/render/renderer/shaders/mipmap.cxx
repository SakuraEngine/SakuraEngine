#include <std/std.hxx>

struct MipGenConstant
{
    float3 texel_size;
};
[[push_constant]]
ConstantBuffer<MipGenConstant> constants;
[[group(1)]]
SamplerState tex_sampler;

Texture1D<float4> src1d;
RWTexture1D<float4> dst1d;

[[numthreads(8, 1, 1)]]
void generate1d([[sv_thread_id]] uint3 dispatch_thread_id)
{
    //DTid is the thread ID * the values from numthreads above and in this case correspond to the pixels location in number of pixels.
    //As a result texcoords (in 0-1 range) will point at the center between the 4 pixels used for the mipmap.
    auto texcoords = constants.texel_size.x * ((float)dispatch_thread_id.x + 0.5f);

    //The samplers linear interpolation will mix the four pixel values to the new pixels color
    float4 color = src1d.SampleLevel(tex_sampler, texcoords, 0.0f);

    //Write the final color into the destination texture.
    dst1d[dispatch_thread_id.x] = color;
}

Texture2D<float4> src2d;
RWTexture2D<float4> dst2d;

[[numthreads(8, 8, 1)]]
void generate2d([[sv_thread_id]] uint3 dispatch_thread_id)
{
    //DTid is the thread ID * the values from numthreads above and in this case correspond to the pixels location in number of pixels.
    //As a result texcoords (in 0-1 range) will point at the center between the 4 pixels used for the mipmap.
    auto texcoords = constants.texel_size.xy * ((float2)dispatch_thread_id.xy + 0.5f);

    //The samplers linear interpolation will mix the four pixel values to the new pixels color
    float4 color = src2d.SampleLevel(tex_sampler, texcoords, 0.0f);

    //Write the final color into the destination texture.
    dst2d[dispatch_thread_id.xy] = color;
}

Texture3D<float4> src3d;
RWTexture3D<float4> dst3d;

[[numthreads(8, 8, 1)]]
void generate3d([[sv_thread_id]] uint3 dispatch_thread_id)
{
    //DTid is the thread ID * the values from numthreads above and in this case correspond to the pixels location in number of pixels.
    //As a result texcoords (in 0-1 range) will point at the center between the 4 pixels used for the mipmap.
    auto texcoords = constants.texel_size.xyz * ((float3)dispatch_thread_id.xyz + 0.5f);

    //The samplers linear interpolation will mix the four pixel values to the new pixels color
    float4 color = src3d.SampleLevel(tex_sampler, texcoords, 0.0f);

    //Write the final color into the destination texture.
    dst3d[dispatch_thread_id.xyz] = color;
}