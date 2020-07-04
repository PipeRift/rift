// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Log.h"

#include <Tracy.hpp>


#define ScopedZone(Name, Color) ZoneScopedNC(Name, 0x##Color)
#define ScopedGameZone(Name) ScopedZone(Name, 459bd1)
#define ScopedGraphicsZone(Name) ScopedZone(Name, 94d145)

#define ScopedStackZone(Color) ZoneScopedC(0x##Color)
#define ScopedStackGameZone() ScopedStackZone(459bd1)
#define ScopedStackGraphicsZone() ScopedStackZone(94d145)

#define Ensure(condition, ...)   \
	if (!(condition))            \
	{                            \
		Log::Error(__VA_ARGS__); \
	}
#define EnsureIf(condition, ...) Ensure(condition, __VA_ARGS__) else
