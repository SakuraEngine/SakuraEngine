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
}
} // namespace skr