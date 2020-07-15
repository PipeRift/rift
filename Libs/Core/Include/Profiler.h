// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Log.h"
#include "Platform/Platform.h"

#include <Tracy.hpp>


namespace VCLang
{
#if TRACY_ENABLE
	inline void ScopedZone(const char* name, u32 color)
	{
		ZoneScopedNC(name, color);
	}

	inline void ScopedZone(const char* name)
	{
		ZoneScopedN(name);
	}

	inline void ScopedZone(u32 color)
	{
		ZoneScopedC(color);
	}
#elif
	inline void ScopedZone(const char* name, u32 color) {}

	inline void ScopedZone(const char* name) {}

	inline void ScopedZone(u32 color) {}
#endif
}	 // namespace VCLang


#define Ensure(condition, ...)   \
	if (!(condition))            \
	{                            \
		Log::Error(__VA_ARGS__); \
	}
#define EnsureIf(condition, ...) Ensure(condition, __VA_ARGS__) else
