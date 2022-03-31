#define WIDTH 3200
#define HEIGHT 2400

struct Pixel
{
    float4 value;
};

[[vk::binding(0, 0)]]
RWStructuredBuffer<float4> buf : register(u0, space0);

[numthreads(32, 32, 1)]
void main(uint3 ThreadID : SV_DispatchThreadID)
{
    if(ThreadID.x >= WIDTH || ThreadID.y >= HEIGHT)
        return;
    float x = float(ThreadID.x) / WIDTH;
    float y = float(ThreadID.y) / HEIGHT;
    float2 uv = float2(x, y);
    float n = 0.0f;
    float2 c = float2(-0.444999992847442626953125f, 0.0f) + ((uv - 0.5f.xx) * 2.3399999141693115234375f);
    float2 z = 0.0f.xx;
    const int M =128;
    for (int i = 0; i < M; i++)
    {
        z = float2((z.x * z.x) - (z.y * z.y), (2.0f * z.x) * z.y) + c;
        if (dot(z, z) > 2.0f)
        {
            break;
        }
        n += 1.0f;
    }

    // we use a simple cosine palette to determine color:
    // http://iquilezles.org/www/articles/palettes/palettes.htm
    float t = float(n) / float(M);
    float3 d = float3(0.3, 0.3 ,0.5);
    float3 e = float3(-0.2, -0.3 ,-0.5);
    float3 f = float3(2.1, 2.0, 3.0);
    float3 g = float3(0.0, 0.1, 0.0);
    float4 color = float4(d + (e * cos(((f * t) + g) * 6.28318023681640625f)), 1.0f);
    buf[(WIDTH * ThreadID.y) + ThreadID.x] = color;
}