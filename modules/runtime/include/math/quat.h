#pragma once
#include "rtm/quatf.h"

#include "type/type.h"

namespace skr
{
namespace math
{
    inline rtm::quatf load(skr_rotator_t r)
    {
        return rtm::quat_from_euler_rh(
        rtm::scalar_deg_to_rad(-r.pitch),
        rtm::scalar_deg_to_rad(r.yaw),
        rtm::scalar_deg_to_rad(r.roll));
    }

    inline skr_rotator_t store(rtm::quatf q)
    {
        // get quaternion components
        float q0 = rtm::quat_get_x(q);
        float q1 = rtm::quat_get_y(q);
        float q2 = rtm::quat_get_z(q);
        float q3 = rtm::quat_get_w(q);

        // calculate euler angles
        float y_angle = atan2(2*(q0*q1 + q2*q3), 1 - 2*(q1*q1 + q2*q2));
        float z_angle = asin(2*(q0*q2 - q3*q1));
        float x_angle = atan2(2*(q0*q3 + q1*q2), 1 - 2*(q2*q2 + q3*q3));
        return skr_rotator_t{
            -rtm::scalar_rad_to_deg(x_angle),
            rtm::scalar_rad_to_deg(y_angle),
            rtm::scalar_rad_to_deg(z_angle)
        };
    }
}
} // namespace skr