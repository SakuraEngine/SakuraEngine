[[vk::binding(0, 0)]]
cbuffer Constants : register(b0)
{
    float4x4 ProjectionMatrix;
};

struct VSIn
{
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

struct VSOut
{
    float4 out_col : COLOR0;
    float2 out_uv  : TEXCOORD0;
};

VSOut main(VSIn input, out float4 position : SV_POSITION)
{
    VSOut output;
    // output.out_pos.xy = 
    //    input.pos.xy * push_constants.invDisplaySize *
    //    float2(2.0, -2.0) + float2(-1.0, 1.0);
    // output.out_pos.zw = float2(0, 1);
    position = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f) );
    output.out_col = input.col;
    output.out_uv = input.uv;
    return output;
}
