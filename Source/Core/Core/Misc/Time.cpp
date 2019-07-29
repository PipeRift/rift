// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "Time.h"
#include "Tools/Profiler.h"


void FrameTime::Tick()
{
	previousTime = currentTime;
	currentTime = DateTime::UtcNow();

	// Avoid too big delta times
	realDeltaTime = Math::Min(0.15f, (currentTime - previousTime).GetTotalSeconds());

	// Apply time dilation
	deltaTime = realDeltaTime * timeDilation;
}

void FrameTime::PostTick()
{
	float extraTimeForFPSCAP = minFrameTime - (DateTime::UtcNow() - currentTime).GetTotalSeconds();
	if (extraTimeForFPSCAP > 0.0f)
	{
		// Cap FPS with a delay
		ScopedZone("Sleep", D15545);
		SDL_Delay(i32(extraTimeForFPSCAP * 1000.f));
	}
}
