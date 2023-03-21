#pragma pack_matrix(row_major)

struct VSIn
{
    float4 position : POSITION;
    float2 texcoord : TEXCOORD0;
    float2 aa : AA;
    float2 clip_uv : UV;
    float2 clip_uv2 : UV_Two;
    float4 color : COLOR;
    nointerpolation float4x4 model : TRANSFORM;
    nointerpolation float4x4 projection : PROJECTION;
    nointerpolation float4x4 draw_data: DRAW_DATA;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float2 aa : AA;
    float2 clip_uv : UV;
    float2 clip_uv2 : UV_Two;
    float4 color : COLOR;
    float4 texture_swizzle : SWIZZLE;
};

VSOut main(const VSIn input)
{
    VSOut output;
    float4 posW = mul(float4(input.position.xyz, 1.0f), input.model);
    float4 posH = mul(posW, input.projection);
    output.position = posH;
    output.texcoord = input.texcoord;
    output.aa = input.aa;
    output.clip_uv = input.clip_uv;
    output.clip_uv2 = input.clip_uv2;
    output.color = input.color;
    output.texture_swizzle = input.draw_data[0];
    // texture_swizzle: none-0 X-1 Y-2 Z-3 W-4
    return output;
}

