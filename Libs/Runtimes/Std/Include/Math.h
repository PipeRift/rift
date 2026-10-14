// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

// Rift functions always return void: inputs are passed by value, outputs by pointer.
// Values are single precision to match Pipe's trigonometric helpers and Rift's Float.
extern "C"
{
	void Abs(float value, float* out);

	void Min(float a, float b, float* out);
	void Max(float a, float b, float* out);
	void Clamp(float value, float minValue, float maxValue, float* out);

	void Sqrt(float value, float* out);
	void Pow(float base, float exponent, float* out);

	void Sin(float value, float* out);
	void Cos(float value, float* out);
	void Tan(float value, float* out);
	void Log(float value, float* out);

	void Floor(float value, float* out);
	void Ceil(float value, float* out);
	void Round(float value, float* out);
}
