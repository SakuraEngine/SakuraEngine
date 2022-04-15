struct PSIn
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

sampler sampler0 : register(s0, space1);
Texture2D texture0 : register(t0);

float4 main(PSIn input) : SV_Target
{
    float4 font_col = texture0.Sample(sampler0, input.uv);
    float4 out_col = input.col * font_col;
    out_col.rgb *= font_col.a;
    return out_col;
}
