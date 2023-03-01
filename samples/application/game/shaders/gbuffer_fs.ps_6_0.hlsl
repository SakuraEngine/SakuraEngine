#pragma pack_matrix(row_major)

struct VSOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    centroid float4 normal : NORMAL;
};

void main(VSOut psIn,     
    out float4 o_color : SV_Target0) : SV_TARGET
{
    float2 uv = psIn.uv;
    o_color = 0.5f + 0.5f * psIn.normal;
}