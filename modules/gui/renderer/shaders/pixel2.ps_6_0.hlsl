#pragma pack_matrix(row_major)

struct VSOut
{
    // ignore SV_POSITION in pixel shader if we dont use it
    // float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float2 aa : AA;
    float2 clip_uv : UV;
    float2 clip_uv2 : UV_Two;
    float4 color : COLOR;
    float4 texture_swizzle : SWIZZLE;
};

[[vk::binding(0, 0)]]
Texture2D color_texture : register(t0, space0);
[[vk::binding(0, 1)]]
SamplerState color_sampler : register(s0, space1);

void main(VSOut input,     
    out float4 o_color : SV_Target0) : SV_TARGET
{
    const float4 texture_color = color_texture.Sample(color_sampler, input.texcoord) * float4(1.f, 1.f, 1.f, 1.f);
    const float swizzle_values[] = { 
        0.f, 
        texture_color.x, texture_color.y, texture_color.z, texture_color.w,
        0.f, 1.f
    };
    float4 swizzled_color = lerp(texture_color, 0.f, saturate(input.texture_swizzle));
    swizzled_color.x += swizzle_values[(int)input.texture_swizzle.x];
    swizzled_color.y += swizzle_values[(int)input.texture_swizzle.y];
    swizzled_color.z += swizzle_values[(int)input.texture_swizzle.z];
    swizzled_color.w += swizzle_values[(int)input.texture_swizzle.w];

    const float visible = (float)all(input.clip_uv == clamp(input.clip_uv, float2(-1.f, -1.f), float2(1.f, 1.f)));
	const float visible2 = (float)all(input.clip_uv2 == clamp(input.clip_uv2, float2(-1.f, -1.f), float2(1.f, 1.f)));
	const float edge = min(1.0, (1.0-abs((input.aa.x * 2.0) - 1.0)) * input.aa.y);
	o_color = input.color * swizzled_color;
	o_color.a = o_color.a * visible * visible2 * edge;
}