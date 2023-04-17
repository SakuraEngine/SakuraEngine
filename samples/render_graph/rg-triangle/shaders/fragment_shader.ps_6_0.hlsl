struct VSOut
{
    // ignore SV_POSITION in pixel shader if we dont use it
    // float4 position : SV_POSITION;
    float3 color : TEXCOORD0;
};

float4 main(VSOut psIn) : SV_TARGET
{
    return float4(psIn.color, 1.f);
}