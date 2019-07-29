// Copyright 2015-2019 Piperift - All rights reserved

#include "Math.h"
#include "Vector.h"

float Math::Atan2(float Y, float X)
{
	//return atan2f(Y,X);
	// atan2f occasionally returns NaN with perfectly valid input (possibly due to a compiler or library bug).
	// We are replacing it with a minimax approximation with a max relative error of 7.15255737e-007 compared to the C library function.
	// On PC this has been measured to be 2x faster than the std C version.

	const float absX = Math::Abs(X);
	const float absY = Math::Abs(Y);
	const bool yAbsBigger = (absY > absX);
	float t0 = yAbsBigger ? absY : absX; // Max(absY, absX)
	float t1 = yAbsBigger ? absX : absY; // Min(absX, absY)

	if (t0 == 0.f)
		return 0.f;

	float t3 = t1 / t0;
	float t4 = t3 * t3;

	static const float c[7] = {
		+7.2128853633444123e-03f,
		-3.5059680836411644e-02f,
		+8.1675882859940430e-02f,
		-1.3374657325451267e-01f,
		+1.9856563505717162e-01f,
		-3.3324998579202170e-01f,
		+1.0f
	};

	t0 = c[0];
	t0 = t0 * t4 + c[1];
	t0 = t0 * t4 + c[2];
	t0 = t0 * t4 + c[3];
	t0 = t0 * t4 + c[4];
	t0 = t0 * t4 + c[5];
	t0 = t0 * t4 + c[6];
	t3 = t0 * t3;

	t3 = yAbsBigger ? (0.5f * PI) - t3 : t3;
	t3 = (X < 0.0f) ? PI - t3 : t3;
	t3 = (Y < 0.0f) ? -t3 : t3;

	return t3;
}

v3 Math::GetSafeScaleReciprocal(const v3& scale, float tolerance /*= SMALL_NUMBER*/)
{
	v3 safeReciprocalScale;
	if (Math::Abs(scale.x) <= tolerance)
		safeReciprocalScale.x = 0.f;
	else
		safeReciprocalScale.x = 1 / scale.x;

	if (Math::Abs(scale.y) <= tolerance)
		safeReciprocalScale.y = 0.f;
	else
		safeReciprocalScale.y = 1 / scale.y;

	if (Math::Abs(scale.z) <= tolerance)
		safeReciprocalScale.z = 0.f;
	else
		safeReciprocalScale.z = 1 / scale.z;

	return safeReciprocalScale;
}
