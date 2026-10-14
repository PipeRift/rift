// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeMath.h>
#include <Std/Math.h>


// The standard library exposes Pipe's math helpers to Rift through out parameters.
extern "C"
{
	void Abs(float value, float* out)
	{
		*out = p::Abs(value);
	}

	void Min(float a, float b, float* out)
	{
		*out = p::Min(a, b);
	}

	void Max(float a, float b, float* out)
	{
		*out = p::Max(a, b);
	}

	void Clamp(float value, float minValue, float maxValue, float* out)
	{
		*out = p::Clamp(value, minValue, maxValue);
	}

	void Sqrt(float value, float* out)
	{
		*out = p::Sqrt(value);
	}

	void Pow(float base, float exponent, float* out)
	{
		*out = p::Pow(base, exponent);
	}

	void Sin(float value, float* out)
	{
		*out = p::Sin(value);
	}

	void Cos(float value, float* out)
	{
		*out = p::Cos(value);
	}

	void Tan(float value, float* out)
	{
		*out = p::Sin(value) / p::Cos(value);
	}

	void Log(float value, float* out)
	{
		*out = p::Log(value);
	}

	void Floor(float value, float* out)
	{
		*out = p::Floor(value);
	}

	void Ceil(float value, float* out)
	{
		*out = p::Ceil(value);
	}

	void Round(float value, float* out)
	{
		*out = p::Round(value);
	}
}
