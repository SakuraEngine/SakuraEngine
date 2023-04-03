struct VSOut
{
     float4 position : SV_POSITION;
     float3 color : TEXCOORD0;
};

float4 main(VSOut psIn) : SV_TARGET
{
    return float4(psIn.color, 1.f);
}