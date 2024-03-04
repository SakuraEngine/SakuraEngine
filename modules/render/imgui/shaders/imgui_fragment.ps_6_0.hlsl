struct PSIn
{
    // ignore SV_POSITION in pixel shader if we dont use it
    // float4 position : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

[[vk::binding(0, 1)]]
Texture2D texture0 : register(t0);
[[vk::binding(1, 1)]]
sampler sampler0 : register(s0, space1);

float4 main(PSIn input) : SV_Target
{
    float4 font_col = texture0.Sample(sampler0, input.uv);
    float4 out_col = input.col * font_col;
    return out_col;
}
