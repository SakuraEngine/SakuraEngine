#include "rtm/vector4f.h"

#include "type/type.h"


namespace skr
{
    namespace math
    {
        inline rtm::vector4f load(skr_float4_t v)
        {
            return rtm::vector_set(v.x, v.y, v.z, v.w);
        }

        inline void store(rtm::vector4f v, skr_float4_t& result)
        {
            rtm::vector_store(v, &result.x);
        }

        inline rtm::vector4f load(skr_float3_t v)
        {
            return rtm::vector_set(v.x, v.y, v.z, 0.f);
        }

        inline void store(rtm::vector4f v, skr_float3_t& result)
        {
            rtm::vector_store3(v, &result.x);
        }

        inline rtm::vector4f load(skr_float2_t v)
        {
            return rtm::vector_set(v.x, v.y, 0.f, 0.f);
        }

        inline void store(rtm::vector4f v, skr_float2_t& result)
        {
            rtm::vector_store2(v, &result.x);
        }
    }
}