#pragma pack_matrix(row_major)

struct VSOut
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float2 aa : AA;
    float2 clip_uv : UV;
    float2 clip_uv2 : UV_Two;
    float4 color : COLOR;
};

[[vk::binding(0, 0)]]
Texture2D color_texture : register(t0, space0);
[[vk::binding(0, 1)]]
SamplerState color_sampler : register(s0, space1);

void main(VSOut input,     
    out float4 o_color : SV_Target0) : SV_TARGET
{
    const float visible = all(input.clip_uv == clamp(input.clip_uv, float2(-1.f, -1.f), float2(1.f, 1.f)));
	const float visible2 = all(input.clip_uv2 == clamp(input.clip_uv2, float2(-1.f, -1.f), float2(1.f, 1.f)));
	const float edge = min(1.0, (1.0-abs((input.aa.x * 2.0) - 1.0)) * input.aa.y);
	o_color = input.color * color_texture.Sample(color_sampler, input.texcoord);
	o_color.w = o_color.w * visible * visible2 * edge;
}