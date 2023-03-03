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
};

struct VSOut
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float2 clip_uv : UV;
    float2 clip_uv2 : UV_Two;
    float4 color : COLOR;
};

VSOut main(const VSIn input)
{
    VSOut output;
    float4 posW = mul(float4(input.position.xyz, 1.0f), input.model);
    float4 posH = mul(posW, input.projection);
    output.position = posH;
    output.color = input.color;
    output.texcoord = input.texcoord;
    return output;
}

