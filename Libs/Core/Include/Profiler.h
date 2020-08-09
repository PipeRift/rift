// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Log.h"
#include "Platform/Platform.h"

#include <Tracy.hpp>


#define Ensure(condition, ...)   \
	if (!(condition))            \
	{                            \
		Log::Error(__VA_ARGS__); \
	}
#define EnsureIf(condition, ...) Ensure(condition, __VA_ARGS__) else
