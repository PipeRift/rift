// Copyright 2015-2019 Piperift - All rights reserved

#include "PlatformTime.h"
#include "Core/Misc/DateTime.h"

u64 PlatformTime::Cycles64()
{
	return date::floor<std::chrono::microseconds>(DateTime::Now().GetTime().time_since_epoch()).count();
}
