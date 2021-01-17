// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Vector.h"


namespace Rift
{
	class Rotator : public Vec<3, float>
	{
		using Vec<3, float>::Vec;

	public:
		float Pitch() const
		{
			return y;
		}
		float Yaw() const
		{
			return z;
		}
		float Roll() const
		{
			return x;
		}

		float& Pitch()
		{
			return y;
		}
		float& Yaw()
		{
			return z;
		}
		float& Roll()
		{
			return x;
		}

		/**
		 * Clamps an angle to the range of [0, 360).
		 *
		 * @param Angle The angle to clamp.
		 * @return The clamped angle.
		 */
		static float ClampAxis(float Angle);

		/**
		 * Clamps an angle to the range of (-180, 180].
		 *
		 * @param Angle The Angle to clamp.
		 * @return The clamped angle.
		 */
		static float NormalizeAxis(float Angle);
	};
}	 // namespace Rift
