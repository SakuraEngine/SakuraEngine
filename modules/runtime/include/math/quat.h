#pragma once
#include "rtm/quatf.h"

#include "type/type.h"

namespace skr
{
namespace math
{
    template<typename T>
    FORCEINLINE T clamp_axis( T Angle )
    {
        // returns Angle in the range (-360,360)
        Angle = fmod(Angle, (T)360.0);

        if (Angle < (T)0.0)
        {
            // shift to [0,360) range
            Angle += (T)360.0;
        }

        return Angle;
    }

    template<typename T>
    FORCEINLINE T normalize_axis( T Angle )
    {
        // returns Angle in the range [0,360)
        Angle = clamp_axis(Angle);

        if (Angle > (T)180.0)
        {
            // shift to (-180,180]
            Angle -= (T)360.0;
        }

        return Angle;
    }
    
    inline rtm::quatf load(skr_rotator_t r)
    {
#if 1
        return rtm::quat_from_euler_rh(
        rtm::scalar_deg_to_rad(-r.pitch),
        rtm::scalar_deg_to_rad(r.yaw),
        rtm::scalar_deg_to_rad(r.roll));
#else
        return rtm::quat_from_euler(
        rtm::scalar_deg_to_rad(r.pitch),
        rtm::scalar_deg_to_rad(r.yaw),
        rtm::scalar_deg_to_rad(r.roll));
#endif
    }

    inline void store(rtm::quatf q, skr_rotator_t& rot)
    {
        // get quaternion components
        float X = rtm::quat_get_x(q);
        float Y = rtm::quat_get_y(q);
        float Z = rtm::quat_get_z(q);
        float W = rtm::quat_get_w(q);
#if 1
    const float SingularityTest = Z * X + W * Y;
	const float RollY = 2.f * (W * Z - X * Y);
	const float RollX = (1.f - 2.f * (Y * Y + Z * Z));

	// reference 
	// http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/

	// this value was found from experience, the above websites recommend different values
	// but that isn't the case for us, so I went through different testing, and finally found the case 
	// where both of world lives happily. 
	const float SINGULARITY_THRESHOLD = 0.4999995f;
	const float RAD_TO_DEG = (180.f / rtm::constants::pi());
	float Pitch, Yaw, Roll;
	if (SingularityTest < -SINGULARITY_THRESHOLD)
	{
		Yaw = -90.f;
		Roll = (rtm::scalar_atan2(RollY, RollX) * RAD_TO_DEG);
		Pitch = normalize_axis(Roll + (2.f * rtm::scalar_atan2(X, W) * RAD_TO_DEG));
	}
	else if (SingularityTest > SINGULARITY_THRESHOLD)
	{
		Yaw = 90.f;
		Roll = (rtm::scalar_atan2(RollY, RollX) * RAD_TO_DEG);
		Pitch = normalize_axis(-Roll + (2.f * rtm::scalar_atan2(X, W) * RAD_TO_DEG));
	}
	else
	{
		Yaw = (rtm::scalar_asin(-2.f * SingularityTest) * RAD_TO_DEG);
		Roll = (rtm::scalar_atan2(RollY, RollX) * RAD_TO_DEG);
		Pitch = (rtm::scalar_atan2(2.f * (W*X - Y*Z), (1.f - 2.f * (X*X + Y*Y))) * RAD_TO_DEG);
	}
    rot = skr_rotator_t{Pitch, Yaw, Roll};
#else
		const float SingularityTest = Z * X - W * Y;
		const float YawY = 2.f * (W * Z + X * Y);
		const float YawX = (1.f - 2.f * (Y * Y + Z * Z));

		// reference 
		// http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
		// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/

		// this value was found from experience, the above websites recommend different values
		// but that isn't the case for us, so I went through different testing, and finally found the case 
		// where both of world lives happily. 
		const float SINGULARITY_THRESHOLD = 0.4999995f;
		const float RAD_TO_DEG = (180.f / rtm::constants::pi());
		float Pitch, Yaw, Roll;
		if (SingularityTest < -SINGULARITY_THRESHOLD)
		{
			Pitch = -90.f;
			Yaw = (rtm::scalar_atan2(YawY, YawX) * RAD_TO_DEG);
			Roll = normalize_axis(-Yaw - (2.f * rtm::scalar_atan2(X, W) * RAD_TO_DEG));
		}
		else if (SingularityTest > SINGULARITY_THRESHOLD)
		{
			Pitch = 90.f;
			Yaw = (rtm::scalar_atan2(YawY, YawX) * RAD_TO_DEG);
			Roll = normalize_axis(Yaw - (2.f * rtm::scalar_atan2(X, W) * RAD_TO_DEG));
		}
		else
		{
			Pitch = (rtm::scalar_asin(2.f * SingularityTest) * RAD_TO_DEG);
			Yaw = (rtm::scalar_atan2(YawY, YawX) * RAD_TO_DEG);
			Roll = (rtm::scalar_atan2(-2.f * (W*X + Y*Z), (1.f - 2.f * (X*X + Y*Y))) * RAD_TO_DEG);
		}
		rot = skr_rotator_t{Pitch, Yaw, Roll};
#endif
    }
}
} // namespace skr