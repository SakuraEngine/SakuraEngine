#pragma pack_matrix(row_major)

struct VSOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    centroid float4 normal : NORMAL;
#ifdef VERTEX_HAS_TANGENT
    float4 tangent : TANGENT;
#endif
};

void main(VSOut psIn, out float4 o_color : SV_Target0) : SV_TARGET
{
    float2 uv = psIn.uv;
    o_color = float4(1.f, 1.f, 1.f, 1.f) * abs(psIn.normal);
}