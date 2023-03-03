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
    // HACK
    float4 divider = float4(input.model[0][0], input.model[0][1], 1.f, 1.f);
    output.position = input.position / divider;
    output.position.w = 1.f;
    // HACK
    output.color = input.color;
    output.texcoord = input.texcoord;
    return output;
}

