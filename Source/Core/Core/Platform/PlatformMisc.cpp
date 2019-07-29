// Copyright 2015-2019 Piperift - All rights reserved

#include "PlatformMisc.h"
#include "Core/Misc/Guid.h"
#include "Core/Misc/DateTime.h"
#include "Core/Misc/Timespan.h"
#include "PlatformTime.h"


void PlatformMisc::CreateGuid(Guid& guid)
{
	static u16 IncrementCounter = 0;

	static DateTime InitialDateTime;

	DateTime EstimatedCurrentDateTime;

	if (IncrementCounter == 0)
	{
		InitialDateTime = DateTime::Now();
		EstimatedCurrentDateTime = InitialDateTime;
	}
	else
	{
		EstimatedCurrentDateTime = DateTime::Now();
	}

	u32 SequentialBits = static_cast<u32>(IncrementCounter++); // Add sequential bits to ensure sequentially generated guids are unique even if Cycles is wrong
	u32 RandBits = Math::Rand() & 0xFFFF; // Add randomness to improve uniqueness across machines

	guid = Guid(RandBits | (SequentialBits << 16), EstimatedCurrentDateTime.GetTicks() >> 32, EstimatedCurrentDateTime.GetTicks() & 0xffffffff, PlatformTime::Cycles());
}
