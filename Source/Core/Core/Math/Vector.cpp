// Copyright 2015-2019 Piperift - All rights reserved

#include "Vector.h"
#include "Core/Platform/PlatformMisc.h"
#include "Core/Serialization/Archive.h"


const v3 v3::Forward{ 0.f, 1.f, 0.f };
const v3 v3::Right  { 1.f, 0.f, 0.f };
const v3 v3::Up     { 0.f, 0.f, 1.f };

v3 Quat::Rotate(const v3& v) const
{
	const v3 q(x, y, z);
	const v3 t = 2.f * glm::cross(q, v);
	return v + (w * t) + glm::cross(q, t);
}

v3 Quat::Unrotate(const v3& v) const
{
	const v3 q(-x, -y, -z); // Inverse
	const v3 t = 2.f * glm::cross(q, v);
	return v + (w * t) + glm::cross(q, t);
}

Rotator Quat::ToRotator() const
{
	const float SingularityTest = z * x - w * y;
	const float YawY = 2.f*(w * z + x * y);
	const float YawX = (1.f - 2.f*(Math::Square(y) + Math::Square(z)));

	// reference
	// http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/

	// this value was found from experience, the above websites recommend different values
	// but that isn't the case for us, so I went through different testing, and finally found the case
	// where both of world lives happily.
	const float SINGULARITY_THRESHOLD = 0.4999995f;
	Rotator RotatorFromQuat;

	if (SingularityTest < -SINGULARITY_THRESHOLD)
	{
		RotatorFromQuat.Pitch() = -90.f;
		RotatorFromQuat.Yaw() = Math::Atan2(YawY, YawX) * Math::RADTODEG;
		RotatorFromQuat.Roll() = Rotator::NormalizeAxis(-RotatorFromQuat.Yaw() - (2.f * Math::Atan2(x, w) * Math::RADTODEG));
	}
	else if (SingularityTest > SINGULARITY_THRESHOLD)
	{
		RotatorFromQuat.Pitch() = 90.f;
		RotatorFromQuat.Yaw() = Math::Atan2(YawY, YawX) * Math::RADTODEG;
		RotatorFromQuat.Roll() = Rotator::NormalizeAxis(RotatorFromQuat.Yaw() - (2.f * Math::Atan2(x, w) * Math::RADTODEG));
	}
	else
	{
		RotatorFromQuat.Pitch() = Math::FastAsin(2.f*(SingularityTest)) * Math::RADTODEG;
		RotatorFromQuat.Yaw()   = Math::Atan2(YawY, YawX) * Math::RADTODEG;
		RotatorFromQuat.Roll()  = Math::Atan2(-2.f*(w * x + y * z), (1.f - 2.f*(Math::Square(x) + Math::Square(y)))) * Math::RADTODEG;
	}

	return RotatorFromQuat;
}

bool Quat::Equals(const Quat& other, float tolerance) const
{
	return Math::NearlyEqual(x, other.x, tolerance) &&
		Math::NearlyEqual(y, other.y, tolerance) &&
		Math::NearlyEqual(z, other.z, tolerance) &&
		Math::NearlyEqual(w, other.w, tolerance);
}

Quat Quat::FromRotator(Rotator rotator)
{
	const float DIVIDE_BY_2 = Math::DEGTORAD / 2.f;
	float SP, SY, SR;
	float CP, CY, CR;

	Math::SinCos(&SP, &CP, rotator.Pitch() * DIVIDE_BY_2);
	Math::SinCos(&SY, &CY, rotator.Yaw()   * DIVIDE_BY_2);
	Math::SinCos(&SR, &CR, rotator.Roll()  * DIVIDE_BY_2);

	Quat RotationQuat{
		 CR * CP*CY + SR * SP*SY,
		 CR * SP*SY - SR * CP*CY,
		-CR * SP*CY - SR * CP*SY,
		 CR * CP*SY - SR * SP*CY
	};

#if ENABLE_NAN_DIAGNOSTIC || DO_CHECK
	// Very large inputs can cause NaN's. Want to catch this here
	ensureMsgf(!RotationQuat.ContainsNaN(), TEXT("Invalid input to FRotator::Quaternion - generated NaN output: %s"), *RotationQuat.ToString());
#endif

	return RotationQuat;
}

Quat Quat::LookAt(const v3& origin, const v3& dest)
{
	return glm::lookAt(origin, dest, v3::Forward);
}

float Rotator::ClampAxis(float Angle)
{
	// returns Angle in the range (-360,360)
	Angle = Math::Mod(Angle, 360.f);

	if (Angle < 0.f)
	{
		// shift to [0,360) range
		Angle += 360.f;
	}

	return Angle;
}

FORCEINLINE float Rotator::NormalizeAxis(float Angle)
{
	// returns Angle in the range [0,360)
	Angle = ClampAxis(Angle);

	if (Angle > 180.f)
	{
		// shift to (-180,180]
		Angle -= 360.f;
	}

	return Angle;
}

v3 v2::xz() const
{
	return v3{ x, 0.f, y };
}

v3 v2::xy() const
{
	return v3{ x, y, 0.f };
}
