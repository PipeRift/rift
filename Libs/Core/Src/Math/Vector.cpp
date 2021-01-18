// Copyright 2015-2020 Piperift - All rights reserved

#include "Math/Vector.h"

#include "Math/Rotator.h"
#include "Platform/PlatformMisc.h"
#include "Serialization/Archive.h"


namespace Rift
{
	const v3 v3::Forward{0.f, 1.f, 0.f};
	const v3 v3::Right{1.f, 0.f, 0.f};
	const v3 v3::Up{0.f, 0.f, 1.f};


	float Rotator::ClampAxis(float angle)
	{
		// returns angle in the range (-360,360)
		angle = Math::Mod(angle, 360.f);
		if (angle < 0.f)
		{
			// shift to [0,360) range
			angle += 360.f;
		}
		return angle;
	}

	float Rotator::NormalizeAxis(float angle)
	{
		// returns angle in the range [0,360)
		angle = ClampAxis(angle);
		if (angle > 180.f)
		{
			// shift to (-180,180]
			angle -= 360.f;
		}

		return angle;
	}

	v3 v2::xz() const
	{
		return v3{x, 0.f, y};
	}

	v3 v2::xy() const
	{
		return v3{x, y, 0.f};
	}
}	 // namespace Rift
