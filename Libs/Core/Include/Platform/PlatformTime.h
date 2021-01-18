// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"

#include <date/date.h>
#include <date/tz.h>

#include <chrono>


namespace Rift
{
	struct PlatformTime
	{
		static u32 Cycles()
		{
			return u32(Cycles64());
		}
		static u64 Cycles64();
	};
}	 // namespace Rift
