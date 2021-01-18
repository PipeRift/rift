// Copyright 2015-2020 Piperift - All rights reserved

#include "Platform/PlatformTime.h"

#include "Misc/DateTime.h"


namespace Rift
{
	u64 PlatformTime::Cycles64()
	{
		return date::floor<std::chrono::microseconds>(DateTime::Now().GetTime().time_since_epoch())
			.count();
	}
}	 // namespace Rift
