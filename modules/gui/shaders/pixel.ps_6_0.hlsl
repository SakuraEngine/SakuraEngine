#pragma pack_matrix(row_major)

struct VSOut
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float2 clip_uv : UV;
    float2 clip_uv2 : UV_Two;
    float4 color : COLOR;
};

void main(VSOut psIn,     
    out float4 o_color : SV_Target0) : SV_TARGET
{
    o_color = psIn.color;
}