#include "rtm/matrix4x4f.h"
#include "type/type.h"


namespace skr
{
    namespace math
    {
        inline rtm::matrix4x4f load(skr_float4x4_t m)
        {
            return {
                rtm::vector_set(m.M[0][0], m.M[0][1], m.M[0][2], m.M[0][3]),
                rtm::vector_set(m.M[1][0], m.M[1][1], m.M[1][2], m.M[1][3]),
                rtm::vector_set(m.M[2][0], m.M[2][1], m.M[2][2], m.M[2][3]),
                rtm::vector_set(m.M[3][0], m.M[3][1], m.M[3][2], m.M[3][3])
            };
        }

        inline void store(rtm::matrix4x4f m, skr_float4x4_t& result)
        {
            rtm::vector_store(m.x_axis, result.M[0]);
            rtm::vector_store(m.y_axis, result.M[1]);
            rtm::vector_store(m.z_axis, result.M[2]);
            rtm::vector_store(m.w_axis, result.M[3]);
        }
    }
}