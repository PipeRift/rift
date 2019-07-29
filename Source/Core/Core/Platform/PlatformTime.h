// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"

#include <chrono>
#include <date/tz.h>
#include <date/date.h>


struct PlatformTime
{
	static FORCEINLINE u32 Cycles() { return u32(Cycles64()); }
	static u64 Cycles64();
};
