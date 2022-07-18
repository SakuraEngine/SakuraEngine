#pragma pack_matrix(row_major)

struct VSOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    centroid float4 normal : NORMAL;
    centroid float4 tangent : TANGENT;
};

void main(VSOut psIn,     
    out float4 o_color : SV_Target0,
    out float4 o_normal : SV_Target1) : SV_TARGET
{
    float2 uv = psIn.uv;
    o_color = float4(1.f, 1.f, 1.f, 1.f);
    o_normal = psIn.normal;
}